/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/empty.hpp>
#include "ametsuchi/tx_presence_cache.hpp"
#include "common/visitor.hpp"
#include "ordering/impl/on_demand_common.hpp"

using namespace iroha;
using namespace iroha::ordering;

OnDemandOrderingGate::OnDemandOrderingGate(
    std::shared_ptr<OnDemandOrderingService> ordering_service,
    std::shared_ptr<transport::OdOsNotification> network_client,
    rxcpp::observable<BlockRoundEventType> events,
    std::shared_ptr<cache::OrderingGateCache> cache,
    std::shared_ptr<shared_model::interface::UnsafeProposalFactory> factory,
    std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
    consensus::Round initial_round,
    logger::Logger log)
    : log_(std::move(log)),
      ordering_service_(std::move(ordering_service)),
      network_client_(std::move(network_client)),
      events_subscription_(events.subscribe([this](auto event) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        visit_in_place(event,
                       [this](const BlockEvent &block_event) {
                         // block committed, increment block round
                         log_->debug("BlockEvent. {}", block_event.round);
                         current_round_ = block_event.round;
                         cache_->remove(block_event.hashes);
                       },
                       [this](const EmptyEvent &empty_event) {
                         // no blocks committed, increment reject round
                         log_->debug("EmptyEvent");
                         current_round_ = empty_event.round;
                       });
        log_->debug("Current: {}", current_round_);

        auto batches = cache_->pop();

        cache_->addToBack(batches);
        if (not batches.empty()) {
          network_client_->onBatches(
              current_round_,
              transport::OdOsNotification::CollectionType{batches.begin(),
                                                          batches.end()});
        }

        // notify our ordering service about new round
        ordering_service_->onCollaborationOutcome(current_round_);

        // request proposal for the current round
        auto proposal = this->processProposalRequest(
            network_client_->onRequestProposal(current_round_));
        // vote for the object received from the network
        proposal_notifier_.get_subscriber().on_next(
            network::OrderingEvent{proposal, current_round_});
      })),
      cache_(std::move(cache)),
      proposal_factory_(std::move(factory)),
      tx_cache_(std::move(tx_cache)),
      current_round_(initial_round) {}

OnDemandOrderingGate::~OnDemandOrderingGate() {
  events_subscription_.unsubscribe();
}

void OnDemandOrderingGate::propagateBatch(
    std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  cache_->addToBack({batch});
  network_client_->onBatches(
      current_round_, transport::OdOsNotification::CollectionType{batch});
}

rxcpp::observable<network::OrderingEvent> OnDemandOrderingGate::onProposal() {
  return proposal_notifier_.get_observable();
}

void OnDemandOrderingGate::setPcs(
    const iroha::network::PeerCommunicationService &pcs) {
  throw std::logic_error(
      "Method is deprecated. PCS observable should be set in ctor");
}

boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingGate::processProposalRequest(
    boost::optional<OnDemandOrderingService::ProposalType> &&proposal) const {
  if (not proposal) {
    return boost::none;
  }
  // no need to check empty proposal
  if (boost::empty(proposal.value()->transactions())) {
    return boost::none;
  }
  return removeReplays(std::move(**std::move(proposal)));
}

boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingGate::removeReplays(
    shared_model::interface::Proposal &&proposal) const {
  auto tx_is_not_processed = [this](const auto &tx) {
    auto tx_result = tx_cache_->check(tx.hash());
    if (not tx_result) {
      // TODO andrei 30.11.18 IR-51 Handle database error
      return false;
    }
    return iroha::visit_in_place(
        *tx_result,
        [](const ametsuchi::tx_cache_status_responses::Missing &) {
          return true;
        },
        [](const auto &status) {
          // TODO nickaleks 21.11.18: IR-1887 log replayed transactions
          // when log is added
          return false;
        });
  };
  auto unprocessed_txs =
      boost::adaptors::filter(proposal.transactions(), tx_is_not_processed);

  auto result = proposal_factory_->unsafeCreateProposal(
      proposal.height(), proposal.createdTime(), unprocessed_txs);

  if (boost::empty(result->transactions())) {
    return boost::none;
  }

  return boost::make_optional<
      std::shared_ptr<shared_model::interface::Proposal>>(std::move(result));
}
