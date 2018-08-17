/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include "interfaces/iroha_internal/proposal.hpp"

using namespace iroha::ordering;

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    CurrentPeers initial_peers,
    rxcpp::observable<CurrentPeers> peers)
    : factory_(std::move(factory)),
      subscription_(peers.start_with(initial_peers).subscribe([&](auto peers) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        connections_.issuer = factory_->create(*peers.issuer);
        connections_.current_consumer =
            factory_->create(*peers.current_consumer);
        connections_.previous_consumer =
            factory_->create(*peers.previous_consumer);
      })) {}

void OnDemandConnectionManager::onTransactions(CollectionType transactions) {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  connections_.current_consumer->onTransactions(transactions);
  connections_.previous_consumer->onTransactions(transactions);
}

boost::optional<OnDemandConnectionManager::ProposalType>
OnDemandConnectionManager::onRequestProposal(transport::RoundType round) {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  return connections_.issuer->onRequestProposal(round);
}
