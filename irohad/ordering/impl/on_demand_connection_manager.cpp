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
      subscription_(peers.subscribe([this](const auto &peers) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        this->initializeConnections(peers);
      })) {
  // using start_with(initial_peers) results in deadlock
  initializeConnections(initial_peers);
}

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

void OnDemandConnectionManager::initializeConnections(
    const CurrentPeers &peers) {
  auto create_assign = [this](auto &ptr, auto &peer) {
    ptr = factory_->create(*peer);
  };

  create_assign(connections_.issuer, peers.issuer);
  create_assign(connections_.current_consumer, peers.current_consumer);
  create_assign(connections_.previous_consumer, peers.previous_consumer);
}
