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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <model/peer.hpp>
#include <rxcpp/rx.hpp>
#include <string>
#include <vector>
#include "ametsuchi/peer_query.hpp"

namespace iroha {

  using namespace std::chrono_literals;
  using PropagationData = GossipPropagationStrategy::PropagationData;

  class MockPeerQuery : public ametsuchi::PeerQuery {
   public:
    MOCK_METHOD0(getLedgerPeers, nonstd::optional<PropagationData>());
  };

  /**
   * @given list of peers and
   *        strategy that emits two peers in some time
   * @when strategy emits this peers
   * @then ensure that all peers is being emitted
   */
  TEST(GossipPropagationStrategyTest, SimpleEmitting) {
    // given
    std::vector<std::string> peersId(23);  // should work for any number
    std::generate(peersId.begin(), peersId.end(), [] {
      static char c = 'a';
      return std::string{c++};
    });
    auto period = 5ms;
    auto amount = 2;
    PropagationData peers;
    std::transform(peersId.begin(), peersId.end(), std::back_inserter(peers),
                   [](auto &s) { return model::Peer(s, pubkey_t{}); });
    auto query = std::make_shared<MockPeerQuery>();
    EXPECT_CALL(*query, getLedgerPeers())
        .WillRepeatedly(testing::Return(peers));
    GossipPropagationStrategy strategy(query, period, amount);

    // when
    PropagationData emitted;
    auto subscriber =
        rxcpp::make_subscriber<std::vector<model::Peer>>([&emitted](auto v) {
          for (const auto &t : v) emitted.push_back(t);
        });
    strategy.emitter().take(peers.size() / amount).subscribe(subscriber);

    // then
    // emitted.size() can be less than peers.size()
    // because emitted.size() must be divisible by amount
    ASSERT_GE(peers.size(), emitted.size());
    ASSERT_FALSE(emitted.size() % amount);
    for (const auto &v : emitted) {
      ASSERT_NE(std::find(peersId.begin(), peersId.end(), v.address),
                peersId.end());
    }
  }

  /**
   * @given no peers and strategy
   * @when strategy emits this peers
   * @then ensure that empty peer list is emitted
   */
  TEST(GossipPropagationStrategyTest, EmptyEmitting) {
    // given
    auto query = std::make_shared<MockPeerQuery>();
    EXPECT_CALL(*query, getLedgerPeers())
        .WillRepeatedly(testing::Return(PropagationData{}));
    GossipPropagationStrategy strategy(query, 1ms, 1);

    // when
    PropagationData emitted;
    auto subscriber =
        rxcpp::make_subscriber<std::vector<model::Peer>>([&emitted](auto v) {
          for (const auto &t : v) emitted.push_back(t);
        });
    strategy.emitter().take(13).subscribe(subscriber);

    // then
    ASSERT_EQ(emitted.size(), 0);
  }

}  // namespace iroha
