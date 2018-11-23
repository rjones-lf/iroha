/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include <chrono>

#include <boost/range/combine.hpp>
#include "interfaces/iroha_internal/proposal.hpp"
#include "ordering/impl/on_demand_common.hpp"

/// The time to wait for the up-to-date propagation data arrives through
/// propagation_params_observable (see constructor) when we got a request for a
/// round that is newer than our propagation data
static constexpr std::chrono::milliseconds kPropagationDataWaitTimeout(10);

using namespace iroha::ordering;

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    rxcpp::observable<PropagationParams> propagation_params_observable)
    : log_(logger::log("OnDemandConnectionManager")),
      factory_(std::move(factory)),
      subscription_(propagation_params_observable.subscribe(
          [this](const auto &propagation_params) {
            // exclusive lock
            std::unique_lock<std::shared_timed_mutex> lock(mutex_);

            this->initializeConnections(propagation_params);
          })) {}

OnDemandConnectionManager::OnDemandConnectionManager(
    std::shared_ptr<transport::OdOsNotificationFactory> factory,
    rxcpp::observable<PropagationParams> propagation_params_observable,
    PropagationParams inital_propagation_params)
    : OnDemandConnectionManager(std::move(factory),
                                propagation_params_observable) {
  // using start_with(inital_propagation_params) results in deadlock
  initializeConnections(inital_propagation_params);
}

void OnDemandConnectionManager::onBatches(const consensus::Round round,
                                          CollectionType batches) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  // check if we have appropriate propagation data:
  if (current_round_ != round) {
    if (round < current_round_) {
      log_->debug(
          "Got a batch to propagate for round {0}, while current round is {1}! "
          "Will send the batch to round {1}.",
          round.toString(),
          current_round_.toString());
    } else if (current_round_ < round) {
      log_->debug(
          "Got a batch to propagate for round {0}, while current round is {1}! "
          "Will wait till current round updates.",
          round.toString(),
          current_round_.toString());
      if (!propagation_data_update_cv_.wait_for(
              lock, kPropagationDataWaitTimeout, [this, &round] {
                return not(this->current_round_ < round);
              })) {
        log_->debug(
            "Reached timeout waiting for the propagation data for at least {} "
            "round. Will drop the batch.",
            round.toString());
        return;
      }
    }
  }

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

  propagate(kCurrentRoundRejectConsumer,
            {current_round_.block_round,
             currentRejectRoundConsumer(current_round_.reject_round)});
  propagate(kNextRoundRejectConsumer,
            {current_round_.block_round + 1, kNextRejectRoundConsumer});
  propagate(kNextRoundCommitConsumer,
            {current_round_.block_round + 2, kNextCommitRoundConsumer});
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
    const PropagationParams &propagation_params) {
  auto create_assign = [this](auto &ptr, auto &peer) {
    ptr = factory_->create(*peer);
  };

  for (auto &&pair :
       boost::combine(connections_.peers, propagation_params.peers)) {
    create_assign(boost::get<0>(pair), boost::get<1>(pair));
  }
  current_round_ = propagation_params.current_round;
}
