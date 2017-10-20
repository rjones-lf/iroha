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
  using OptPeer = GossipPropagationStrategy::OptPeer;
  using std::chrono::steady_clock;

  GossipPropagationStrategy::GossipPropagationStrategy(
      std::shared_ptr<ametsuchi::PeerQuery> query,
      std::chrono::milliseconds period, uint32_t amount)
      : query(query),
        emitent(rxcpp::observable<>::interval(steady_clock::now(), period)
                    .map([this, amount](int) {
                      PropagationData vec;
                      vec.reserve(amount);
                      OptPeer element;
                      for (auto &v : vec) {
                        element = this->visit();
                        if (!element) break;
                        v = *element;
                      }
                      return vec;
                    })) {}

  rxcpp::observable<PropagationData> GossipPropagationStrategy::emitter() {
    return emitent;
  }

  void GossipPropagationStrategy::initQueue(const PropagationData &data) {
    std::vector<decltype(non_visited)::value_type> v(data.size());
    std::iota(v.begin(), v.end(), 0);
    std::random_shuffle(v.begin(), v.end());
    non_visited = {v.begin(), v.end()};
  }

  OptPeer GossipPropagationStrategy::visit() {
    auto data_opt = query->getLedgerPeers();
    if (!data_opt || data_opt->size() == 0) {
      return nonstd::nullopt;
    }
    const auto data = *data_opt;
    if (non_visited.empty()) {
      initQueue(data);
    }

    auto el = data[non_visited.top()];
    non_visited.pop();
    return el;
  }

}  // namespace iroha
