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

#include "multi_sig_transactions/gossip_propagation_strategy.hpp"
#include <numeric>
#include <vector>

namespace iroha {

using PropagationData = PropagationStrategy::PropagationData;

GossipPropagationStrategy::GossipPropagationStrategy(PropagationData data,
                                                     uint32_t frequency,
                                                     uint32_t amount)
    : data(data),
      emitent(
          rxcpp::observable<>::interval(std::chrono::steady_clock::now(),
                                        std::chrono::milliseconds(frequency))
              .map([this, amount](int v) {
                std::vector<decltype(data)::value_type> vec(amount);
                std::for_each(vec.begin(), vec.end(), [this](auto &el) {
                  if (this->non_visited.empty())
                    this->initQueue();
                  el = this->data[this->non_visited.top()];
                  this->non_visited.pop();
                });
                return vec;
              })) {}

rxcpp::observable<PropagationData> GossipPropagationStrategy::emitter() {
  return emitent;
}

void GossipPropagationStrategy::initQueue() {
  std::vector<decltype(non_visited)::value_type> v(data.size());
  std::iota(v.begin(), v.end(), 0);
  std::random_shuffle(v.begin(), v.end());
  non_visited = std::priority_queue<decltype(non_visited)::value_type>{
      v.begin(), v.end()};
}

} // namespace iroha