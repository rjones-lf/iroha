/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_GOSSIP_PROPAGATION_STRATEGY_PARAMS_HPP
#define IROHA_GOSSIP_PROPAGATION_STRATEGY_PARAMS_HPP

#include <chrono>

#include <boost/optional.hpp>

namespace iroha {

  /**
   * This structure provides configuration parameters for propagation strategy
   */
  struct GossipPropagationStrategyParams {
    // TODO: IR-1317 @l4l (02/05/18) magics should be replaced with options via
    // cli parameters
    static constexpr std::chrono::milliseconds kDefaultPeriod =
        std::chrono::seconds(5);
    static constexpr uint32_t kDefaultAmount = 2;

    /// period of emitting data in ms
    std::chrono::milliseconds period{kDefaultPeriod};

    /// amount of data (peers) emitted per once
    uint32_t amount{kDefaultAmount};
  };

  using OptGossipPropagationStrategyParams = boost::optional<GossipPropagationStrategyParams>;

}  // namespace iroha

#endif  // IROHA_GOSSIP_PROPAGATION_STRATEGY_PARAMS_HPP
