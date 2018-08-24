/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"

using namespace iroha;
using namespace iroha::ordering;

OnDemandOrderingGate::OnDemandOrderingGate(
    std::shared_ptr<OnDemandOrderingService> ordering_service,
    std::shared_ptr<transport::OdOsNotification> network_client,
    rxcpp::observable<BlockRoundEventType> events,
    std::unique_ptr<shared_model::interface::UnsafeProposalFactory> factory,
    transport::RoundType initial_round)
    : ordering_service_(std::move(ordering_service)),
      network_client_(std::move(network_client)),
      events_subscription_(events.subscribe([this](auto event) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        visit_in_place(event,
                       [this](BlockEvent block) {
                         // block committed, increment block round
                         current_round_ = {block.height, 1};
                       },
                       [this](EmptyEvent empty) {
                         // no blocks committed, increment reject round
                         current_round_ = {current_round_.first,
                                           current_round_.second + 1};
                       });

        // notify our ordering service about new round
        ordering_service_->onCollaborationOutcome(current_round_);

        // request proposal for the current round
        auto proposal = network_client_->onRequestProposal(current_round_);

        // vote for the object received from the network
        proposal_notifier_.get_subscriber().on_next(
            std::move(proposal).value_or_eval([&] {
              return proposal_factory_->unsafeCreateProposal(
                  current_round_.first, current_round_.second, {});
            }));
      })),
      proposal_factory_(std::move(factory)),
      current_round_(initial_round) {}

void OnDemandOrderingGate::propagateTransaction(
    std::shared_ptr<const shared_model::interface::Transaction> transaction)
    const {
  throw std::logic_error("Not implemented");
}

void OnDemandOrderingGate::propagateBatch(
    const shared_model::interface::TransactionBatch &batch) const {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  network_client_->onTransactions(current_round_, batch.transactions());
}

rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingGate::on_proposal() {
  return proposal_notifier_.get_observable();
}

void OnDemandOrderingGate::setPcs(
    const iroha::network::PeerCommunicationService &pcs) {
  throw std::logic_error("Not implemented");
}
