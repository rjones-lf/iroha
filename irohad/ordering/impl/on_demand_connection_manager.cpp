/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include <boost/range/combine.hpp>
#include "interfaces/iroha_internal/proposal.hpp"

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

void OnDemandConnectionManager::onTransactions(transport::Round round,
                                               CollectionType transactions) {
  // shared lock
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  const PeerType types[] = {kCurrentRoundRejectConsumer,
                            kNextRoundRejectConsumer,
                            kNextRoundCommitConsumer};
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
  const transport::Round rounds[] = {
      {round.block_round, round.reject_round + 2},
      {round.block_round + 1, 2},
      {round.block_round + 2, 1}};

  for (auto &&pair : boost::combine(types, rounds)) {
    auto &&round = boost::get<1>(pair);

    log_->debug(
        "onTransactions, round[{}, {}]", round.block_round, round.reject_round);

    connections_.peers[boost::get<0>(pair)]->onTransactions(round,
                                                            transactions);
  }
}

boost::optional<OnDemandConnectionManager::ProposalType>
OnDemandConnectionManager::onRequestProposal(transport::Round round) {
  // shared lock
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
