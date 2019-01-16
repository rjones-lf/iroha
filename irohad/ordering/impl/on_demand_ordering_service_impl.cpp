/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

#include <unordered_set>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/size.hpp>
#include "ametsuchi/tx_presence_cache.hpp"
#include "ametsuchi/tx_presence_cache_utils.hpp"
#include "common/visitor.hpp"
#include "datetime/time.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"

using namespace iroha;
using namespace iroha::ordering;

OnDemandOrderingServiceImpl::OnDemandOrderingServiceImpl(
    size_t transaction_limit,
    std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
        proposal_factory,
    std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
    size_t number_of_proposals,
    const consensus::Round &initial_round,
    logger::Logger log)
    : transaction_limit_(transaction_limit),
      number_of_proposals_(number_of_proposals),
      proposal_factory_(std::move(proposal_factory)),
      tx_cache_(std::move(tx_cache)),
      log_(std::move(log)) {
  onCollaborationOutcome(initial_round);
}

// -------------------------| OnDemandOrderingService |-------------------------

void OnDemandOrderingServiceImpl::onCollaborationOutcome(
    consensus::Round round) {
  log_->info("onCollaborationOutcome => {}", round);
  // exclusive write lock
  std::lock_guard<std::shared_timed_mutex> guard(lock_);
  log_->debug("onCollaborationOutcome => write lock is acquired");

  packNextProposals(round);
  tryErase();
}

// ----------------------------| OdOsNotification |-----------------------------

void OnDemandOrderingServiceImpl::onBatches(consensus::Round round,
                                            CollectionType batches) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  log_->info("onBatches => collection size = {}, {}", batches.size(), round);

  auto unprocessed_batches =
      boost::adaptors::filter(batches, [this](const auto &batch) {
        log_->info("check batch {} for already processed transactions",
                   batch->reducedHash().hex());
        return not this->batchAlreadyProcessed(*batch);
      });
  auto it = current_proposals_.find(round);
  if (it == current_proposals_.end()) {
    it =
        std::find_if(current_proposals_.begin(),
                     current_proposals_.end(),
                     [&round](const auto &p) {
                       auto request_reject_round = round.reject_round;
                       auto reject_round = p.first.reject_round;
                       return request_reject_round == reject_round
                           or (request_reject_round >= 2 and reject_round >= 2);
                     });
    BOOST_ASSERT_MSG(it != current_proposals_.end(),
                     "No place to store the batches!");
    log_->debug("onBatches => collection will be inserted to {}", it->first);
  }
  std::for_each(unprocessed_batches.begin(),
                unprocessed_batches.end(),
                [&it](auto &obj) { it->second.push(std::move(obj)); });
  log_->debug("onBatches => collection is inserted");
}

boost::optional<OnDemandOrderingServiceImpl::ProposalType>
OnDemandOrderingServiceImpl::onRequestProposal(consensus::Round round) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  auto proposal = proposal_map_.find(round);
  // space between '{}' and 'returning' is not missing, since either nothing, or
  // NOT with space is printed
  log_->debug("onRequestProposal, {}, {}returning a proposal.",
              round,
              (proposal == proposal_map_.end()) ? "NOT " : "");
  if (proposal != proposal_map_.end()) {
    return clone(*proposal->second);
  } else {
    return boost::none;
  }
}

// ---------------------------------| Private |---------------------------------

void OnDemandOrderingServiceImpl::packNextProposals(
    const consensus::Round &round) {
  auto close_round = [this](consensus::Round round) {
    log_->debug("close {}", round);

    auto it = current_proposals_.find(round);
    if (it != current_proposals_.end()) {
      log_->debug("proposal found");
      if (not it->second.empty()) {
        proposal_map_.emplace(round, emitProposal(round));
        log_->debug("packNextProposal: data has been fetched for {}", round);
        round_queue_.push(round);
      }
      current_proposals_.erase(it);
    }
  };

  auto open_round = [this](consensus::Round round) {
    log_->debug("open {}", round);
    current_proposals_[round];
  };

  /*
   * The possible cases can be visualised as a diagram, where:
   * o - current round, x - next round, v - target round
   *
   *   0 1 2
   * 0 o x v
   * 1 x v .
   * 2 v . .
   *
   * Reject case:
   *
   *   0 1 2 3
   * 0 . o x v
   * 1 x v . .
   * 2 v . . .
   *
   * (0,1) - current round. Round (0,2) is closed for transactions.
   * Round (0,3) is now receiving transactions.
   * Rounds (1,) and (2,) do not change.
   *
   * Commit case:
   *
   *   0 1 2
   * 0 . . .
   * 1 o x v
   * 2 x v .
   * 3 v . .
   *
   * (1,0) - current round. The diagram is similar to the initial case.
   */

  // close next reject round
  close_round({round.block_round, round.reject_round + 1});

  if (round.reject_round == kFirstRejectRound) {
    // new block round
    close_round({round.block_round + 1, round.reject_round});

    // remove current queues
    current_proposals_.clear();
    // initialize the 3 diagonal rounds from the commit case diagram
    open_round({round.block_round + 1, kNextRejectRoundConsumer});
    open_round({round.block_round + 2, kNextCommitRoundConsumer});
  }

  // new reject round
  open_round(
      {round.block_round, currentRejectRoundConsumer(round.reject_round)});
}

OnDemandOrderingServiceImpl::ProposalType
OnDemandOrderingServiceImpl::emitProposal(const consensus::Round &round) {
  log_->debug("Mutable proposal generation, {}", round);

  TransactionBatchType batch;
  std::vector<std::shared_ptr<shared_model::interface::Transaction>> collection;
  std::unordered_set<std::string> inserted;

  // outer method should guarantee availability of at least one transaction in
  // queue, also, code shouldn't fetch all transactions from queue. The rest
  // will be lost.
  auto &current_proposal = current_proposals_[round];
  while (current_proposal.try_pop(batch)
         and collection.size() < transaction_limit_
         and inserted.insert(batch->reducedHash().hex()).second) {
    collection.insert(
        std::end(collection),
        std::make_move_iterator(std::begin(batch->transactions())),
        std::make_move_iterator(std::end(batch->transactions())));
  }
  log_->debug("Number of transactions in proposal = {}", collection.size());
  log_->debug("Number of lost transactions = {}",
              current_proposal.unsafe_size());

  auto txs = collection | boost::adaptors::indirected;
  return proposal_factory_->unsafeCreateProposal(
      round.block_round, iroha::time::now(), txs);
}

void OnDemandOrderingServiceImpl::tryErase() {
  if (round_queue_.size() >= number_of_proposals_) {
    auto &round = round_queue_.front();
    proposal_map_.erase(round);
    log_->info("tryErase: erased {}", round);
    round_queue_.pop();
  }
}

bool OnDemandOrderingServiceImpl::batchAlreadyProcessed(
    const shared_model::interface::TransactionBatch &batch) {
  auto tx_statuses = tx_cache_->check(batch);
  if (not tx_statuses) {
    // TODO andrei 30.11.18 IR-51 Handle database error
    log_->warn("Check tx presence database error. Batch: {}", batch);
    return true;
  }
  // if any transaction is commited or rejected, batch was already processed
  // Note: any_of returns false for empty sequence
  return std::any_of(
      tx_statuses->begin(), tx_statuses->end(), [this](const auto &tx_status) {
        if (iroha::ametsuchi::isAlreadyProcessed(tx_status)) {
          log_->warn("Duplicate transaction: {}",
                     iroha::ametsuchi::getHash(tx_status).hex());
          return true;
        }
        return false;
      });
}
