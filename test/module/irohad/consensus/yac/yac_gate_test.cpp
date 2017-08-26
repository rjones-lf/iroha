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

#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/irohad/simulator/simulator_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"

#include <memory>
#include <rxcpp/rx-observable.hpp>
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "framework/test_subscriber.hpp"

using namespace iroha::consensus::yac;
using namespace iroha::network;
using namespace iroha::simulator;
using namespace framework::test_subscriber;

#include <iostream>
using namespace std;

using ::testing::Return;
using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;

TEST(YacGateTest, YacGateSubscriptionTest) {
  cout << "----------| BlockCreator (block)=> YacGate (vote)=> "
      "HashGate (commit) => YacGate => on_commit() |----------" << endl;

  // expected values
  YacHash expected_hash("proposal", "block");
  iroha::model::Block expected_block;
  expected_block.created_ts = 100500;
  expected_block.sigs.push_back({});
  VoteMessage message;
  message.hash = expected_hash;
  message.signature = expected_block.sigs.front();
  CommitMessage commit_message({message});
  auto expected_commit = rxcpp::observable<>::just(commit_message);

  // yac consensus
  unique_ptr<HashGate> hash_gate = make_unique<MockHashGate>();
  auto hash_gate_raw = hash_gate.get();

  EXPECT_CALL(*static_cast<MockHashGate *>(hash_gate_raw),
              vote(expected_hash, _)).Times(1);

  EXPECT_CALL(*static_cast<MockHashGate *>(hash_gate_raw),
              on_commit()).WillOnce(Return(expected_commit));

  // generate order of peers
  unique_ptr<YacPeerOrderer> peer_orderer =
      make_unique<MockYacPeerOrderer>();
  auto peer_orderer_raw = peer_orderer.get();

  EXPECT_CALL(*static_cast<MockYacPeerOrderer *>(peer_orderer_raw),
              getOrdering(_))
      .WillOnce(Return(ClusterOrdering({mk_peer("fake_node")})));

  // make hash from block
  shared_ptr<YacHashProvider> hash_provider =
      make_shared<MockYacHashProvider>();

  EXPECT_CALL(
      *static_cast<MockYacHashProvider *>(hash_provider.get()), makeHash(_))
      .WillOnce(Return(expected_hash));

  // make blocks
  auto block_creator = make_shared<MockBlockCreator>();
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(rxcpp::observable<>::just(expected_block)));

  // Block loader
  auto block_loader = make_shared<MockBlockLoader>();

  YacGateImpl gate(std::move(hash_gate), std::move(peer_orderer),
                   hash_provider, block_creator, block_loader);

  // verify that yac gate emit expected block
  auto gate_wrapper = make_test_subscriber<CallExact>(gate.on_commit(), 1);
  gate_wrapper.subscribe([expected_block](auto block) {
    ASSERT_EQ(block, expected_block);
  });

  ASSERT_TRUE(gate_wrapper.validate());
}

TEST(YacGateTest, YacGateSubscribtionTestFailCase) {
  cout << "----------| Fail case of retrieving cluster order  |----------"
       << endl;

  // expected values
  YacHash expected_hash("proposal", "block");
  iroha::model::Block expected_block;
  expected_block.created_ts = 100500;
  VoteMessage message;
  message.hash = expected_hash;
  CommitMessage commit_message({message});
  auto expected_commit = rxcpp::observable<>::just(commit_message);

  // yac consensus
  unique_ptr<HashGate> hash_gate =
      make_unique<MockHashGate>();
  auto hash_gate_raw = hash_gate.get();

  EXPECT_CALL(*static_cast<MockHashGate *>(hash_gate_raw),
              vote(_, _)).Times(0);

  EXPECT_CALL(*static_cast<MockHashGate *>(hash_gate_raw),
              on_commit()).Times(0);

  // generate order of peers
  unique_ptr<YacPeerOrderer> peer_orderer =
      make_unique<MockYacPeerOrderer>();
  auto peer_orderer_raw = peer_orderer.get();

  EXPECT_CALL(*static_cast<MockYacPeerOrderer *>(peer_orderer_raw),
              getOrdering(_))
      .WillOnce(Return(nonstd::nullopt));

  // make hash from block
  shared_ptr<YacHashProvider> hash_provider =
      make_shared<MockYacHashProvider>();

  EXPECT_CALL(
      *static_cast<MockYacHashProvider *>(hash_provider.get()), makeHash(_))
      .WillOnce(Return(expected_hash));

  // make blocks
  auto block_creator = make_shared<MockBlockCreator>();
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(rxcpp::observable<>::just(expected_block)));

  // Block loader
  auto block_loader = make_shared<MockBlockLoader>();

  YacGateImpl gate(std::move(hash_gate), std::move(peer_orderer),
                   hash_provider, block_creator, block_loader);
}
