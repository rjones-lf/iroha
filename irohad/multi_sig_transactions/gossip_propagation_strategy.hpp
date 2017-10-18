/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_GOSSIP_PROPAGATION_STRATEGY_HPP
#define IROHA_GOSSIP_PROPAGATION_STRATEGY_HPP

#include <chrono>
#include <nonstd/optional.hpp>
#include <rxcpp/rx.hpp>
#include "ametsuchi/peer_query.hpp"
#include "multi_sig_transactions/mst_propagation_strategy.hpp"

namespace iroha {

  /**
   * This class provides strategy for propagation states in network
   * Choose unique amount of peers each period of time
   */
  class GossipPropagationStrategy : public PropagationStrategy {
   public:
    using PeerProvider = std::shared_ptr<ametsuchi::PeerQuery>;
    using OptPeer = nonstd::optional<PropagationData::value_type>;
    /**
     * Initialize strategy with
     * @param query is a provider of peer list
     * @param period of emitting to observable in ms
     * @param amount of peers emitted per once
     */
    GossipPropagationStrategy(PeerProvider query,
                              std::chrono::milliseconds period,
                              uint32_t amount);

    // ------------------| PropagationStrategy override |------------------

    rxcpp::observable<PropagationData> emitter() override;

    // --------------------------| end override |---------------------------
   private:
    /**
     * Peer provider
     */
    PeerProvider query;
    /**
     * Cache of peer provider's data
     */
    PropagationData last_data;
    rxcpp::observable<PropagationData> emitent;
    /**
     * Queue that represents peers indexes of data that have
     * not been emitted yet
     */
    std::vector<size_t> non_visited;

    /**
     * Fill a queue with random ordered list of peers
     * @return true if sucessful
     */
    bool initQueue();

    /**
     * Visit next element of non_visited
     * @return following peer
     */
    OptPeer visit();
  };
}  // namespace iroha

#endif  // IROHA_GOSSIP_PROPAGATION_STRATEGY_HPP
