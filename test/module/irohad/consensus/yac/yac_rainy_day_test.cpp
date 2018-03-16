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

#include <gmock/gmock.h>
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;

/**
 * @given yac consensus with four peers
 * @when two peers vote for one hash and two for another
 * @then commit does not happen, instead send_reject is triggered on transport
 */
TEST_F(YacTest, InvalidCaseWhenNotReceiveSupermajority) {
  auto my_peers = std::vector<iroha::model::Peer>(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(my_peers.size());
  EXPECT_CALL(*network, send_vote(_, _)).Times(my_peers.size());

  EXPECT_CALL(*timer, deny()).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  YacHash hash1("proposal_hash", "block_hash");
  YacHash hash2("proposal_hash", "block_hash2");
  yac->vote(hash1, my_order.value());

  for (auto i = 0; i < 2; ++i) {
    yac->on_vote(create_vote(hash1, std::to_string(i)));
  };
  for (auto i = 2; i < 4; ++i) {
    yac->on_vote(create_vote(hash2, std::to_string(i)));
  };
}

/**
 * @given yac consensus
 * @when 2 peers vote for one hash and 2 for another, but yac_crypto verify
 * always returns false
 * @then reject is not propagated
 */
TEST_F(YacTest, InvalidCaseWhenDoesNotVerify) {
  auto my_peers = std::vector<iroha::model::Peer>(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  EXPECT_CALL(*network, send_reject(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>()))
      .WillRepeatedly(Return(false));
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(false));

  YacHash hash1("proposal_hash", "block_hash");
  YacHash hash2("proposal_hash", "block_hash2");

  for (auto i = 0; i < 2; ++i) {
    yac->on_vote(create_vote(hash1, std::to_string(i)));
  };
  for (auto i = 2; i < 4; ++i) {
    yac->on_vote(create_vote(hash2, std::to_string(i)));
  };
}

/**
 * @given yac consensus with 6 peers
 * @when on_reject happens due to 2 peers vote for one hash and 3 peers vote for
 * another and then last 6th peer votes for any hash, he directly receives
 * reject message, because on_reject already happened
 * @then reject message will be called in total 7 times (peers size + 1 who
 * receives reject directly)
 */
TEST_F(YacTest, ValidCaseWhenReceiveOnVoteAfterReject) {
  size_t peers_number = 6;
  auto my_peers = std::vector<iroha::model::Peer>(
      {default_peers.begin(), default_peers.begin() + peers_number});
  ASSERT_EQ(peers_number, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _))
      .Times(my_peers.size() + 1);  // $(peers.size()) sendings done during
                                    // multicast + 1 for single peer, who votes
                                    // after reject happened
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(1);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).WillOnce(Return(true));
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  YacHash hash1("proposal_hash", "block_hash");
  YacHash hash2("proposal_hash", "block_hash2");

  std::vector<VoteMessage> votes;
  for (size_t i = 0; i < peers_number / 2; ++i) {
    votes.push_back(
        create_vote(hash1, my_order->getPeers().at(i).pubkey.to_string()));
  };
  for (size_t i = peers_number / 2; i < peers_number - 1; ++i) {
    votes.push_back(
        create_vote(hash2, my_order->getPeers().at(i).pubkey.to_string()));
  };

  for (const auto &vote : votes) {
    yac->on_vote(vote);
  }

  yac->on_reject(RejectMessage(votes));
  yac->on_vote(
      create_vote(hash1, my_order->getPeers().back().pubkey.to_string()));
}
