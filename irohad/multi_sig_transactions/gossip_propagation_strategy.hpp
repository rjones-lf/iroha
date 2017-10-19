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

#include "mst_propagation_strategy.hpp"
#include <queue>
#include <rxcpp/rx.hpp>

namespace iroha {

/**
 * This class provides strategy for propagation states in network
 * Choose unique amount of peers each period of time
 */
class GossipPropagationStrategy : public PropagationStrategy {
public:
  /**
   * Initialize strategy with
   * @param data full list of peers; TODO: replace with provider of peer list
   * @param period of emitting to observable in ms
   * @param amount of peers emitted per once
   */
  GossipPropagationStrategy(PropagationData data, uint32_t period,
                            uint32_t amount);
  // ------------------| PropagationStrategy override |------------------

  rxcpp::observable<PropagationData> emitter() override;

  // --------------------------| end override |---------------------------
private:
  const PropagationData data;
  rxcpp::observable<PropagationData> emitent;
  /**
   * Queue that represents peers indexes of data that have not been emitted yet
   */
  std::priority_queue<size_t> non_visited;

  /**
   * Fill a queue with random ordered list of peers
   */
  void initQueue();
};
} // namespace iroha

#endif // IROHA_GOSSIP_PROPAGATION_STRATEGY_HPP
