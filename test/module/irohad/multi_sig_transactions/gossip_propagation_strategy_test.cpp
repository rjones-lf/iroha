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
#include <iterator>
#include <memory>
#include <numeric>
#include <rxcpp/rx.hpp>
#include <string>
#include <vector>
#include "ametsuchi/peer_query.hpp"
#include "model/peer.hpp"

using namespace iroha;

using namespace std::chrono_literals;
using PropagationData = GossipPropagationStrategy::PropagationData;
using Peers = std::vector<model::Peer>;

class MockPeerQuery : public ametsuchi::PeerQuery {
 public:
  MOCK_METHOD0(getLedgerPeers, nonstd::optional<PropagationData>());
};

/**
 * Generates peers with empty pub keys
 * @param ids generated addresses of peers
 * @param amount for generation
 * @return generated peers
 */
PropagationData generate(std::vector<std::string> &ids, size_t num) {
  ids.resize(num);
  std::iota(ids.begin(), ids.end(), 'a');
  PropagationData peers;
  std::transform(
      ids.begin(), ids.end(), std::back_inserter(peers), [](auto &s) {
        return model::Peer(s, pubkey_t{});
      });
  return peers;
}

/**
 * Perform subscription and the emitting from created strategy
 * @param data retrieved from the PeerProvider
 * @param period of the strategy
 * @param take is amount taken from the strategy emitter
 * @return emitted data
 */
PropagationData subscribe_and_emit(nonstd::optional<PropagationData> data,
                                   std::chrono::milliseconds period,
                                   uint32_t amount,
                                   uint32_t take) {
  auto query = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*query, getLedgerPeers()).WillRepeatedly(testing::Return(data));
  GossipPropagationStrategy strategy(query, period, amount);

  PropagationData emitted;
  auto subscriber = rxcpp::make_subscriber<Peers>([&emitted](auto v) {
    std::copy(v.begin(), v.end(), std::back_inserter(emitted));
  });
  strategy.emitter().take(take).subscribe(subscriber);

  return emitted;
}

/**
 * @given list of peers and
 *        strategy that emits two peers
 * @when strategy emits this peers
 * @then ensure that all peers are being emitted
 */
TEST(GossipPropagationStrategyTest, EmittingAllPeers) {
  auto peers_size = 23, amount = 2, take = peers_size / amount;
  std::vector<std::string> peersId;
  PropagationData peers = generate(peersId, peers_size);

  auto emitted = subscribe_and_emit(peers, 1ms, amount, take);

  // emitted.size() can be less than peers.size()
  ASSERT_GE(peers.size(), emitted.size());
  // because emitted size should be increased by amount at once
  ASSERT_FALSE(emitted.size() % amount);
  std::for_each(emitted.begin(), emitted.end(), [&peersId](const auto &v) {
    ASSERT_NE(std::find(peersId.begin(), peersId.end(), v.address),
              peersId.end());
  });
}

/**
 * @given list of peers and
 *        strategy that emits two peers
 * @when strategy emits more than peers available
 * @then ensure that there's been emitted peers
 */
TEST(GossipPropagationStrategyTest, EmitEvenOnOddPeers) {
  auto peers_size = 11, amount = 2, take = 6;
  std::vector<std::string> peersId;
  PropagationData peers = generate(peersId, peers_size);

  auto emitted = subscribe_and_emit(peers, 1ms, amount, take);

  ASSERT_EQ(emitted.size(), take * amount);
  ASSERT_LE(peers.size(), emitted.size());
  std::for_each(emitted.begin(), emitted.end(), [&peersId](const auto &v) {
    ASSERT_NE(std::find(peersId.begin(), peersId.end(), v.address),
              peersId.end());
  });
}

/**
 * @given no peers and strategy
 * @when strategy emits this peers
 * @then ensure that empty peer list is emitted
 */
TEST(GossipPropagationStrategyTest, EmptyEmitting) {
  auto emitted = subscribe_and_emit(PropagationData{}, 1ms, 1, 13);
  ASSERT_EQ(emitted.size(), 0);
}

/**
 * @given nullopt emitting instead of peers list and strategy
 * @when strategy emits this peers
 * @then ensure that empty peer list is emitted
 */
TEST(GossipPropagationStrategyTest, ErrorEmitting) {
  auto emitted = subscribe_and_emit(nonstd::nullopt, 1ms, 1, 13);
  ASSERT_EQ(emitted.size(), 0);
}
