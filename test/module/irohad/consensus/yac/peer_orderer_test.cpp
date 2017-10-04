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

#include <iostream>
#include <memory>
#include <vector>

#include "consensus/yac/impl/peer_orderer_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::consensus::yac;

using namespace std;
using ::testing::Return;

class YacPeerOrdererTest : public ::testing::Test {
 public:
  YacPeerOrdererTest() : orderer(make_shared<MockPeerQuery>()) {}

  void SetUp() override {
    wsv = make_shared<MockPeerQuery>();
    orderer = PeerOrdererImpl(wsv);
  }

  std::vector<iroha::model::Peer> peers = [] {
    std::vector<iroha::model::Peer> result;
    for (size_t i = 1; i <= 4; ++i) {
      result.push_back(iroha::consensus::yac::mk_peer(std::to_string(i)));
    }
    return result;
  }();

  shared_ptr<MockPeerQuery> wsv;
  PeerOrdererImpl orderer;
};

TEST_F(YacPeerOrdererTest, PeerOrdererInitialOrderWhenInvokeNormalCase) {
  cout << "----------| InitialOrder() => valid object |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(peers));
  auto order = orderer.getInitialOrdering();
  ASSERT_EQ(order.value().getPeers(), peers);
}

TEST_F(YacPeerOrdererTest, PeerOrdererInitialOrderWhenInvokeFailCase) {
  cout << "----------| InitialOrder() => nullopt case |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(nonstd::nullopt));
  auto order = orderer.getInitialOrdering();
  ASSERT_EQ(order, nonstd::nullopt);
}

TEST_F(YacPeerOrdererTest, PeerOrdererOrderingWhenInvokeNormalCase) {
  cout << "----------| Order() => valid object |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(peers));
  auto order = orderer.getOrdering(YacHash());
  ASSERT_EQ(order.value().getPeers().size(), peers.size());
}

TEST_F(YacPeerOrdererTest, PeerOrdererOrderingWhenInvokeFaillCase) {
  cout << "----------| Order() => nullopt case |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(nonstd::nullopt));
  auto order = orderer.getOrdering(YacHash());
  ASSERT_EQ(order, nonstd::nullopt);
}
