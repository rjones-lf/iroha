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

#include <rxcpp/rx.hpp>
#include <queue>
#include "mst_propagation_strategy.hpp"

namespace iroha {

  /**
   * Interface provides strategy for propagation states in network
   */
  class GossipPropagationStrategy : public PropagationStrategy {
   public:
    /*
     * Initialize strategy with:
     * @param full list of peers; TODO: replace with provider of peer list
     * @param frequency of emitting to observable in ms
     * @param amount of peers emited per once
     */
     GossipPropagationStrategy(PropagationData, uint32_t, uint32_t);
     // ------------------| PropagationStrategy override |------------------

     rxcpp::observable<PropagationData> emitter() override;

     // --------------------------| end override |---------------------------
   private:
     const PropagationData data;
     rxcpp::observable<PropagationData> emitent;
     std::priority_queue<size_t> non_visited;

     void initQueue();
  };
  } // namespace iroha

#endif // IROHA_GOSSIP_PROPAGATION_STRATEGY_HPP
