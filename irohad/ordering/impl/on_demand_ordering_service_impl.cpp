/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

#include <unordered_set>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include "datetime/time.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"

using namespace iroha::ordering;

/**
 * First round after successful committing block
 */
const iroha::consensus::RejectRoundType kFirstRound = 1;

OnDemandOrderingServiceImpl::OnDemandOrderingServiceImpl(
    size_t transaction_limit,
    std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
    size_t number_of_proposals,
    const consensus::Round &initial_round)
    : transaction_limit_(transaction_limit),
      number_of_proposals_(number_of_proposals),
      proposal_factory_(std::move(proposal_factory)),
      log_(logger::log("OnDemandOrderingServiceImpl")) {
  onCollaborationOutcome(initial_round);
}

// -------------------------| OnDemandOrderingService |-------------------------

void OnDemandOrderingServiceImpl::onCollaborationOutcome(
    consensus::Round round) {
  log_->info("onCollaborationOutcome => round[{}, {}]",
             round.block_round,
             round.reject_round);
  // exclusive write lock
  std::lock_guard<std::shared_timed_mutex> guard(lock_);
  log_->info("onCollaborationOutcome => write lock is acquired");

  packNextProposals(round);
  tryErase();
}

// ----------------------------| OdOsNotification |-----------------------------

void OnDemandOrderingServiceImpl::onBatches(consensus::Round round,
                                            CollectionType batches) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  log_->info("onBatches => collection size = {}, round[{}, {}]",
             batches.size(),
             round.block_round,
             round.reject_round);

  auto it = current_proposals_.find(round);
  if (it != current_proposals_.end()) {
    std::for_each(batches.begin(), batches.end(), [&it](auto &obj) {
      it->second.push(std::move(obj));
    });
    log_->info("onTransactions => collection is inserted");
  }
}

boost::optional<OnDemandOrderingServiceImpl::ProposalType>
OnDemandOrderingServiceImpl::onRequestProposal(consensus::Round round) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  auto proposal = proposal_map_.find(round);
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
    auto it = current_proposals_.find(round);
    if (it != current_proposals_.end()) {
      if (not it->second.empty()) {
        proposal_map_.emplace(round, emitProposal(round));
        log_->info("packNextProposal: data has been fetched for round[{}, {}]",
                   round.block_round,
                   round.reject_round);
        round_queue_.push(round);
      }
      current_proposals_.erase(it);
    }
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

  if (round.reject_round == kFirstRound) {
    // new block round
    close_round({round.block_round + 1, round.reject_round});

    // remove current queues
    current_proposals_.clear();
    // initialize the 3 diagonal rounds from the commit case diagram
    for (uint32_t i = 0; i <= 2; ++i) {
      current_proposals_[{round.block_round + i, round.reject_round + 2 - i}];
    }
  } else {
    // new reject round
    current_proposals_[{round.block_round, round.reject_round + 2}];
  }
}

OnDemandOrderingServiceImpl::ProposalType
OnDemandOrderingServiceImpl::emitProposal(const consensus::Round &round) {
  log_->info("Mutable proposal generation, round[{}, {}]",
             round.block_round,
             round.reject_round);

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
  log_->info("Number of transactions in proposal = {}", collection.size());

  auto txs = collection | boost::adaptors::indirected;
  return proposal_factory_->unsafeCreateProposal(
      round.block_round, iroha::time::now(), txs);
}

void OnDemandOrderingServiceImpl::tryErase() {
  if (round_queue_.size() >= number_of_proposals_) {
    auto &round = round_queue_.front();
    proposal_map_.erase(round);
    log_->info("tryErase: erased round[{}, {}]",
               round.block_round,
               round.reject_round);
    round_queue_.pop();
  }
}
