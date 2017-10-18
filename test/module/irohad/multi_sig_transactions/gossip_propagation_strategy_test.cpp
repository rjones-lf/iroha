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
#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <model/peer.hpp>
#include <rxcpp/rx.hpp>
#include <string>
#include <vector>

namespace iroha {

TEST(GossipPropagationStrategyTest, SimpleEmitting) {
  std::vector<std::string> peersId{"a", "b", "c", "d"};
  std::vector<model::Peer> peers;
  std::transform(peersId.begin(), peersId.end(), std::back_inserter(peers),
                 [](auto &s) { return model::Peer(s, pubkey_t{}); });
  GossipPropagationStrategy strategy(peers, 5, 2);
  decltype(peers) emitted;
  strategy.emitter().take(2).subscribe(rxcpp::make_subscriber<std::vector<model::Peer>>([&emitted](auto v){
    for (const auto& t: v) emitted.push_back(t);
  }));
  for (const auto &v: emitted)
    ASSERT_NE(std::find(peersId.begin(), peersId.end(), v.address), peersId.end());
}

} // namespace iroha