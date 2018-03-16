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
#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;
using namespace std;

TEST_F(YacTest, ValidCaseWhenReceiveSupermajority) {
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

  EXPECT_CALL(*network, send_commit(_, _)).Times(my_peers.size());
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(my_peers.size());

  EXPECT_CALL(*timer, deny()).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  YacHash my_hash("proposal_hash", "block_hash");
  yac->vote(my_hash, my_order.value());

  for (auto i = 0; i < 3; ++i) {
    yac->on_vote(create_vote(my_hash, std::to_string(i)));
  };
}

TEST_F(YacTest, ValidCaseWhenReceiveCommit) {
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

  YacHash my_hash("proposal_hash", "block_hash");
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 1);
  wrapper.subscribe(
      [my_hash](auto val) { ASSERT_EQ(my_hash, val.votes.at(0).hash); });

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(my_peers.size());

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  EXPECT_CALL(*crypto, verify(An<CommitMessage>()))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  yac->vote(my_hash, my_order.value());

  auto votes = std::vector<VoteMessage>();

  for (auto i = 0; i < 4; ++i) {
    votes.push_back(create_vote(my_hash, std::to_string(i)));
  };
  yac->on_commit(CommitMessage(votes));
  ASSERT_TRUE(wrapper.validate());
}

TEST_F(YacTest, ValidCaseWhenReceiveCommitTwice) {
  auto my_peers = std::vector<iroha::model::Peer>(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;
  EXPECT_CALL(*timer, deny()).Times(2);

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  YacHash my_hash("proposal_hash", "block_hash");
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 1);
  wrapper.subscribe(
      [my_hash](auto val) { ASSERT_EQ(my_hash, val.votes.at(0).hash); });

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(my_peers.size());

  EXPECT_CALL(*crypto, verify(An<CommitMessage>()))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  yac->vote(my_hash, my_order.value());

  auto votes = std::vector<VoteMessage>();

  // first commit
  for (auto i = 0; i < 3; ++i) {
    votes.push_back(create_vote(my_hash, std::to_string(i)));
  };
  yac->on_commit(CommitMessage(votes));

  // second commit
  for (auto i = 1; i < 4; ++i) {
    votes.push_back(create_vote(my_hash, std::to_string(i)));
  };
  yac->on_commit(CommitMessage(votes));

  ASSERT_TRUE(wrapper.validate());
}

TEST_F(YacTest, ValidCaseWhenSoloConsensus) {
  auto my_peers = std::vector<iroha::model::Peer>({default_peers.at(0)});
  ASSERT_EQ(1, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  EXPECT_CALL(*network, send_commit(_, _)).Times(my_peers.size());
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(my_peers.size());

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  EXPECT_CALL(*crypto, verify(An<CommitMessage>()))
      .Times(1)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>()))
      .Times(1)
      .WillRepeatedly(Return(true));

  YacHash my_hash("proposal_hash", "block_hash");

  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 1);
  wrapper.subscribe(
      [my_hash](auto val) { ASSERT_EQ(my_hash, val.votes.at(0).hash); });

  yac->vote(my_hash, my_order.value());

  auto vote_message = create_vote(my_hash, std::to_string(0));

  yac->on_vote(vote_message);

  auto commit_message = CommitMessage({vote_message});

  yac->on_commit(commit_message);

  ASSERT_TRUE(wrapper.validate());
}

TEST_F(YacTest, ValidCaseWhenVoteAfterCommit) {
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
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  EXPECT_CALL(*crypto, verify(An<CommitMessage>()))
      .Times(1)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).Times(0);

  YacHash my_hash("proposal_hash", "block_hash");

  std::vector<VoteMessage> votes;

  for (auto i = 0; i < 3; ++i) {
    votes.push_back(create_vote(my_hash, std::to_string(i)));
  };
  yac->on_commit(CommitMessage(votes));

  yac->vote(my_hash, my_order.value());
}
