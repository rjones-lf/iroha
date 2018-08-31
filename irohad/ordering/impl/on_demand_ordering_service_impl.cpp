/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

#include <unordered_set>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include "backend/protobuf/proposal.hpp"
#include "backend/protobuf/transaction.hpp"
#include "datetime/time.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"

using namespace iroha::ordering;

/**
 * First round after successful committing block
 */
const iroha::ordering::transport::RejectRoundType kFirstRound = 1;

OnDemandOrderingServiceImpl::OnDemandOrderingServiceImpl(
    size_t transaction_limit,
    size_t number_of_proposals,
    const transport::Round &initial_round)
    : transaction_limit_(transaction_limit),
      number_of_proposals_(number_of_proposals),
      log_(logger::log("OnDemandOrderingServiceImpl")) {
  onCollaborationOutcome(initial_round);
}

// -------------------------| OnDemandOrderingService |-------------------------

void OnDemandOrderingServiceImpl::onCollaborationOutcome(
    transport::Round round) {
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

void OnDemandOrderingServiceImpl::onTransactions(transport::Round round,
                                                 CollectionType transactions) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  log_->info("onTransactions => collections size = {}, round[{}, {}]",
             transactions.size(),
             round.block_round,
             round.reject_round);

  auto it = current_proposals_.find(round);
  if (it != current_proposals_.end()) {
    std::for_each(transactions.begin(), transactions.end(), [&it](auto &obj) {
      it->second.push(std::move(obj));
    });
    log_->info("onTransactions => collection is inserted");
  }
}

boost::optional<OnDemandOrderingServiceImpl::ProposalType>
OnDemandOrderingServiceImpl::onRequestProposal(transport::Round round) {
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
    const transport::Round &round) {
  auto close_round = [this](transport::Round round) {
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
OnDemandOrderingServiceImpl::emitProposal(const transport::Round &round) {
  iroha::protocol::Proposal proto_proposal;
  proto_proposal.set_height(round.block_round);
  proto_proposal.set_created_time(iroha::time::now());
  log_->info("Mutable proposal generation, round[{}, {}]",
             round.block_round,
             round.reject_round);

  TransactionType current_tx;
  using ProtoTxType = shared_model::proto::Transaction;
  std::vector<TransactionType> collection;
  std::unordered_set<std::string> inserted;

  // outer method should guarantee availability of at least one transaction in
  // queue, also, code shouldn't fetch all transactions from queue. The rest
  // will be lost.
  auto &current_proposal = current_proposals_[round];
  while (current_proposal.try_pop(current_tx)
         and collection.size() < transaction_limit_
         and inserted.insert(current_tx->hash().hex()).second) {
    collection.push_back(std::move(current_tx));
  }
  log_->info("Number of transactions in proposal = {}", collection.size());
  auto proto_txes = collection | boost::adaptors::transformed([](auto &tx) {
                      return static_cast<const ProtoTxType &>(*tx);
                    });
  boost::for_each(proto_txes, [&proto_proposal](auto &&proto_tx) {
    *(proto_proposal.add_transactions()) = std::move(proto_tx.getTransport());
  });

  return std::make_unique<shared_model::proto::Proposal>(
      std::move(proto_proposal));
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
