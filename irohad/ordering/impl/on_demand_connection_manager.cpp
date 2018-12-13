/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include <boost/range/combine.hpp>
#include "interfaces/iroha_internal/proposal.hpp"
#include "ordering/impl/on_demand_common.hpp"

using namespace iroha::ordering;

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    rxcpp::observable<CurrentPeers> peers)
    : log_(logger::log("OnDemandConnectionManager")),
      factory_(std::move(factory)),
      subscription_(peers.subscribe([this](const auto &peers) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        this->initializeConnections(peers);
      })) {}

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    rxcpp::observable<CurrentPeers> peers,
    CurrentPeers initial_peers)
    : OnDemandConnectionManager(std::move(factory), peers) {
  // using start_with(initial_peers) results in deadlock
  initializeConnections(initial_peers);
}

void OnDemandConnectionManager::onBatches(consensus::Round round,
                                          CollectionType batches) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  /*
   * Transactions are always sent to the round after the next round (+2)
   * There are 3 possibilities - next reject in the current round, first reject
   * in the next round, and first commit in the round after the next round
   * This can be visualised as a diagram, where:
   * o - current round, x - next round, v - target round
   *
   *   0 1 2
   * 0 o x v
   * 1 x v .
   * 2 v . .
   */

  auto propagate = [this, batches](PeerType type, consensus::Round round) {
    log_->debug(
        "onTransactions, round[{}, {}]", round.block_round, round.reject_round);

    connections_.peers[type]->onBatches(round, batches);
  };

  propagate(
      kCurrentRoundRejectConsumer,
      {round.block_round, currentRejectRoundConsumer(round.reject_round)});
  propagate(kNextRoundRejectConsumer,
            {round.block_round + 1, kNextRejectRoundConsumer});
  propagate(kNextRoundCommitConsumer,
            {round.block_round + 2, kNextCommitRoundConsumer});
}

boost::optional<OnDemandConnectionManager::ProposalType>
OnDemandConnectionManager::onRequestProposal(consensus::Round round) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  log_->debug("onRequestProposal, round[{}, {}]",
              round.block_round,
              round.reject_round);

  return connections_.peers[kIssuer]->onRequestProposal(round);
}

void OnDemandConnectionManager::initializeConnections(
    const CurrentPeers &peers) {
  auto create_assign = [this](auto &ptr, auto &peer) {
    ptr = factory_->create(*peer);
  };

  for (auto &&pair : boost::combine(connections_.peers, peers.peers)) {
    create_assign(boost::get<0>(pair), boost::get<1>(pair));
  }
}
