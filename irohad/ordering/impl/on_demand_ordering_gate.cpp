/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include "common/visitor.hpp"

using namespace iroha;
using namespace iroha::ordering;

OnDemandOrderingGate::OnDemandOrderingGate(
    std::shared_ptr<OnDemandOrderingService> ordering_service,
    std::shared_ptr<transport::OdOsNotification> network_client,
    rxcpp::observable<BlockRoundEventType> events,
    std::shared_ptr<cache::OrderingGateCache> cache,
    std::unique_ptr<shared_model::interface::UnsafeProposalFactory> factory,
    consensus::Round initial_round)
    : ordering_service_(std::move(ordering_service)),
      network_client_(std::move(network_client)),
      events_subscription_(events.subscribe([this](auto event) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        visit_in_place(event,
                       [this](const BlockEvent &block_event) {
                         // block committed, increment block round
                         current_round_ = block_event.round;
                         cache_->remove(block_event.hashes);
                       },
                       [this](const EmptyEvent &empty) {
                         // no blocks committed, increment reject round
                         current_round_ = {current_round_.block_round,
                                           current_round_.reject_round + 1};
                       });

        auto batches = cache_->pop();

        cache_->addToBack(batches);
        network_client_->onBatches(current_round_,
                                   transport::OdOsNotification::CollectionType{
                                       batches.begin(), batches.end()});

        // notify our ordering service about new round
        ordering_service_->onCollaborationOutcome(current_round_);

        // request proposal for the current round
        auto proposal = network_client_->onRequestProposal(current_round_);

        // vote for the object received from the network
        proposal_notifier_.get_subscriber().on_next(
            std::move(proposal).value_or_eval([&] {
              return proposal_factory_->unsafeCreateProposal(
                  current_round_.block_round, current_round_.reject_round, {});
            }));
      })),
      cache_(std::move(cache)),
      proposal_factory_(std::move(factory)),
      current_round_(initial_round) {}

void OnDemandOrderingGate::propagateBatch(
    std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  cache_->addToBack({batch});
  network_client_->onBatches(
      current_round_, transport::OdOsNotification::CollectionType{batch});
}

rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingGate::on_proposal() {
  return proposal_notifier_.get_observable();
}

void OnDemandOrderingGate::setPcs(
    const iroha::network::PeerCommunicationService &pcs) {
  throw std::logic_error(
      "Method is deprecated. PCS observable should be set in ctor");
}
