/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

#include <unordered_set>

#include <boost/optional.hpp>
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
#include "logger/logger.hpp"

using namespace iroha;
using namespace iroha::ordering;
using TransactionBatchType = transport::OdOsNotification::TransactionBatchType;

OnDemandOrderingServiceImpl::OnDemandOrderingServiceImpl(
    size_t transaction_limit,
    std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
        proposal_factory,
    std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
    logger::LoggerPtr log,
    size_t number_of_proposals,
    const consensus::Round &initial_round)
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
  tryErase(round);
}

// ----------------------------| OdOsNotification |-----------------------------

void OnDemandOrderingServiceImpl::onBatches(CollectionType batches) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  log_->info("onBatches => collection size = {}", batches.size());

  auto unprocessed_batches =
      boost::adaptors::filter(batches, [this](const auto &batch) {
        log_->info("check batch {} for already processed transactions",
                   batch->reducedHash().hex());
        return not this->batchAlreadyProcessed(*batch);
      });
  std::for_each(
      unprocessed_batches.begin(),
      unprocessed_batches.end(),
      [this](auto &obj) { current_round_batches_.insert(std::move(obj)); });
  log_->debug("onBatches => collection is inserted");
}

boost::optional<
    std::shared_ptr<const OnDemandOrderingServiceImpl::ProposalType>>
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
    return proposal->second;
  } else {
    return boost::none;
  }
}

// ---------------------------------| Private |---------------------------------

/**
 * Get transactions from the given batches queue. Does not break batches -
 * continues getting all the transactions from the ongoing batch until the
 * required amount is collected.
 * @tparam Lambda - type of side effect function for batches
 * @param requested_tx_amount - amount of transactions to get
 * @param batch_collection - the collection to get transactions from
 * @param discarded_txs_amount - the amount of discarded txs
 * @param batch_operation - side effect function to be performed on each
 * inserted batch. Passed pointer could be modified
 * @return transactions
 */
template <typename Lambda>
static std::vector<std::shared_ptr<shared_model::interface::Transaction>>
getTransactions(size_t requested_tx_amount,
                detail::BatchSetType &batch_collection,
                boost::optional<size_t &> discarded_txs_amount,
                Lambda batch_operation) {
  std::vector<std::shared_ptr<shared_model::interface::Transaction>> collection;

  auto it = batch_collection.begin();
  for (;
       it != batch_collection.end() and collection.size() < requested_tx_amount;
       ++it) {
    collection.insert(std::end(collection),
                      std::begin((*it)->transactions()),
                      std::end((*it)->transactions()));
    batch_operation(*it);
  }

  if (discarded_txs_amount) {
    *discarded_txs_amount = 0;
    for (; it != batch_collection.end(); ++it) {
      *discarded_txs_amount += boost::size((*it)->transactions());
    }
  }
  batch_collection.clear();

  return collection;
}

void OnDemandOrderingServiceImpl::packNextProposals(
    const consensus::Round &round) {
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

  size_t discarded_txs_amount;
  auto get_transactions = [this, &discarded_txs_amount](auto &queue,
                                                        auto lambda) {
    return getTransactions(
        transaction_limit_, queue, discarded_txs_amount, lambda);
  };

  auto now = iroha::time::now();
  auto generate_proposal = [this, now, &discarded_txs_amount](
                               consensus::Round round, const auto &txs) {
    auto proposal = proposal_factory_->unsafeCreateProposal(
        round.block_round, now, txs | boost::adaptors::indirected);
    proposal_map_.emplace(round, std::move(proposal));
    log_->debug(
        "packNextProposal: data has been fetched for {}. "
        "Number of transactions in proposal = {}. Discarded {} "
        "transactions.",
        round,
        txs.size(),
        discarded_txs_amount);
  };

  if (not current_round_batches_.empty()) {
    auto txs = get_transactions(current_round_batches_, [this](auto &batch) {
      next_round_batches_.insert(std::move(batch));
    });

    if (not txs.empty() and round.reject_round != kFirstRejectRound) {
      generate_proposal({round.block_round, round.reject_round + 1}, txs);
    }
  }

  if (not next_round_batches_.empty()
      and round.reject_round == kFirstRejectRound) {
    auto txs = get_transactions(next_round_batches_, [](auto &) {});

    if (not txs.empty()) {
      generate_proposal({round.block_round, kNextRejectRoundConsumer}, txs);
      generate_proposal({round.block_round + 1, kNextCommitRoundConsumer}, txs);
    }
  }
}

void OnDemandOrderingServiceImpl::tryErase(
    const consensus::Round &current_round) {
  auto current_proposal =
      std::lower_bound(proposal_map_.begin(),
                       proposal_map_.end(),
                       current_round,
                       [](const auto &map_item, const auto &round) {
                         return map_item.first < round;
                       });

  auto proposal_range_size = boost::size(
      boost::make_iterator_range(proposal_map_.begin(), current_proposal));

  while (proposal_range_size > number_of_proposals_) {
    BOOST_ASSERT(proposal_map_.begin()->first < current_round);
    log_->info("tryErase: erasing {}", proposal_map_.begin()->first);
    proposal_map_.erase(proposal_map_.begin());
    --proposal_range_size;
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
