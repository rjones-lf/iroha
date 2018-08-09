/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include "interfaces/iroha_internal/proposal.hpp"

using namespace iroha::ordering;

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    rxcpp::observable<CurrentPeers> peers)
    : factory_(std::move(factory)),
      subscription_(peers.subscribe([&](auto peers) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        issuer_ = factory_->create(*peers.issuer);
        current_consumer_ = factory_->create(*peers.current_consumer);
        previous_consumer_ = factory_->create(*peers.previous_consumer);
      })) {}

// OdOsNotification

void OnDemandConnectionManager::onTransactions(CollectionType transactions) {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  current_consumer_->onTransactions(transactions);
  previous_consumer_->onTransactions(transactions);
}

boost::optional<OnDemandConnectionManager::ProposalType>
OnDemandConnectionManager::onRequestProposal(transport::RoundType round) {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  return issuer_->onRequestProposal(round);
}
