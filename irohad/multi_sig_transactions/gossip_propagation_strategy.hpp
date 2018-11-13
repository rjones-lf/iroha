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

#include <boost/optional.hpp>
#include <chrono>
#include <mutex>

#include "ametsuchi/peer_query_factory.hpp"
#include "multi_sig_transactions/gossip_propagation_strategy_params.hpp"
#include "multi_sig_transactions/mst_propagation_strategy.hpp"

namespace iroha {

  /**
   * This class provides strategy for propagation states in network
   * Emits exactly (or zero iff provider is empty) amount of peers
   * at some period
   * note: it can be inconsistent with the peer provider
   */
  class GossipPropagationStrategy : public PropagationStrategy {
   public:
    using PeerProviderFactory = std::shared_ptr<ametsuchi::PeerQueryFactory>;
    using OptPeer = boost::optional<PropagationData::value_type>;
    /**
     * Initialize strategy with
     * @param peer_factory is a provider of peer list
     * @param emit_worker is the coordinator for the data emitting
     * @param params configuration parameters
     */
    GossipPropagationStrategy(PeerProviderFactory peer_factory,
                              rxcpp::observe_on_one_worker emit_worker,
                              const GossipPropagationStrategyParams &params);

    ~GossipPropagationStrategy();

    // ------------------| PropagationStrategy override |------------------

    rxcpp::observable<PropagationData> emitter() override;

    // --------------------------| end override |---------------------------
   private:
    /**
     * Source of peers for propagation
     */
    PeerProviderFactory peer_factory;

    /**
     * Cache of peer provider's data
     */
    PropagationData last_data;

    /**
     * Queue that contains non-emitted indexes of peers
     */
    std::vector<size_t> non_visited;

    /**
     * Worker that performs internal loop handling
     */
    rxcpp::observe_on_one_worker emit_worker;

    /*
     * Observable for the emitting propagated data
     */
    rxcpp::observable<PropagationData> emitent;

    /*
     * Mutex for handling observable stopping
     */
    std::mutex m;

    /**
     * Fill a queue with a random ordered list of peers
     * @return true if query to PeerProvider was successful
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
