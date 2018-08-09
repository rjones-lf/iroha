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
    const transport::RoundType &initial_round)
    : transaction_limit_(transaction_limit),
      number_of_proposals_(number_of_proposals),
      current_proposal_(std::make_pair(
          initial_round, tbb::concurrent_queue<TransactionType>())),
      log_(logger::log("OnDemandOrderingServiceImpl")) {}

// -------------------------| OnDemandOrderingService |-------------------------

void OnDemandOrderingServiceImpl::onCollaborationOutcome(RoundOutput outcome) {
  auto current_round = current_proposal_.first;
  log_->info("onCollaborationOutcome => round[{}, {}]",
             current_round.first,
             current_round.second);
  // exclusive write lock
  std::lock_guard<std::shared_timed_mutex> guard(lock_);
  log_->info("onCollaborationOutcome => write lock is acquired");

  packNextProposal(outcome);
  tryErase();
}

// ----------------------------| OdOsNotification |-----------------------------

void OnDemandOrderingServiceImpl::onTransactions(CollectionType transactions) {
  // read lock
  std::shared_lock<std::shared_timed_mutex> guard(lock_);
  log_->info("onTransactions => collections size = {}", transactions.size());

  std::for_each(transactions.begin(), transactions.end(), [this](auto &obj) {
    current_proposal_.second.push(std::move(obj));
  });
  log_->info("onTransactions => collection is inserted");
}

boost::optional<OnDemandOrderingServiceImpl::ProposalType>
OnDemandOrderingServiceImpl::onRequestProposal(transport::RoundType round) {
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

void OnDemandOrderingServiceImpl::packNextProposal(RoundOutput outcome) {
  log_->info("pack next proposal...size of queue = {}",
             current_proposal_.second.unsafe_size());
  if (not current_proposal_.second.empty()) {
    proposal_map_.insert(
        std::make_pair(current_proposal_.first, emitProposal()));
    log_->info("packNextProposal: data has been fetched");
  }

  round_queue_.push(current_proposal_.first);
  log_->info("packNextProposal: pushed in queue");

  auto current_round = current_proposal_.first;
  decltype(current_round) next_round;
  switch (outcome) {
    case RoundOutput::kCommitProposal:
      next_round = current_round;
      break;
    case RoundOutput::kCommitBlock:
      next_round = std::make_pair(current_round.first + 1, kFirstRound);
      break;
    case RoundOutput::kReject:
      next_round =
          std::make_pair(current_round.first, current_round.second + 1);
      break;
  }

  log_->info("nextRound is: [{}, {}]", next_round.first, next_round.second);

  current_proposal_.first = next_round;
  current_proposal_.second.clear();  // operator = of tbb::queue is closed
}

OnDemandOrderingServiceImpl::ProposalType
OnDemandOrderingServiceImpl::emitProposal() {
  iroha::protocol::Proposal proto_proposal;
  proto_proposal.set_height(current_proposal_.first.first);
  proto_proposal.set_created_time(iroha::time::now());
  log_->info("Mutable proposal generation");

  TransactionType current_tx;
  using ProtoTxType = shared_model::proto::Transaction;
  std::vector<TransactionType> collection;
  std::unordered_set<std::string> inserted;

  // outer method should guarantee availability of at least one transaction in
  // queue, also, code shouldn't fetch all transactions from queue. The rest
  // will be lost.
  while (current_proposal_.second.try_pop(current_tx)
         and collection.size() < transaction_limit_
         and inserted.insert(current_tx->hash().hex()).second) {
    collection.emplace_back(std::move(current_tx));
  }
  log_->info("Number of transaction in proposal  = {}", collection.size());
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
    proposal_map_.erase(round_queue_.front());
    round_queue_.pop();
  }
}
