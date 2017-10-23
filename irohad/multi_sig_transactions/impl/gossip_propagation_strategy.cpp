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
#include <boost/assert.hpp>
#include <numeric>
#include <vector>

namespace iroha {

  using PropagationData = PropagationStrategy::PropagationData;
  using OptPeer = GossipPropagationStrategy::OptPeer;
  using PeerProvider = GossipPropagationStrategy::PeerProvider;
  using std::chrono::steady_clock;

  GossipPropagationStrategy::GossipPropagationStrategy(
      PeerProvider query, std::chrono::milliseconds period, uint32_t amount)
      : query(query),
        emitent(rxcpp::observable<>::interval(steady_clock::now(), period)
                    .map([this, amount](int) {
                      PropagationData vec;
                      for (uint32_t i = 0; i < amount; i++) {
                        OptPeer element = this->visit();
                        if (not element) break;
                        vec.push_back(*element);
                      }
                      return vec;
                    })) {}

  rxcpp::observable<PropagationData> GossipPropagationStrategy::emitter() {
    return emitent;
  }

  bool GossipPropagationStrategy::initQueue() {
    return static_cast<bool>(
        query->getLedgerPeers() |
            [this](PropagationData data) -> boost::optional<int> {
          last_data = data;
          if (data.size() == 0) return {};
          this->non_visited.resize(this->last_data.size());
          std::iota(this->non_visited.begin(), this->non_visited.end(), 0);
          std::random_shuffle(this->non_visited.begin(),
                              this->non_visited.end());
          return {0}; // any value on optional derefernce gives true
        });
  }

  OptPeer GossipPropagationStrategy::visit() {
    if (non_visited.empty() and not initQueue()) {
      return {};
    }
    // either initQueue exits from method or non_visited non-empty
    BOOST_ASSERT(not non_visited.empty());

    auto el = last_data[non_visited.back()];
    non_visited.pop_back();
    return el;
  }

}  // namespace iroha
