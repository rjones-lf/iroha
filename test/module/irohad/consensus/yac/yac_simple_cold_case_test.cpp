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

#ifndef IROHA_YAC_SIMPLE_CASE_TEST_HPP
#define IROHA_YAC_SIMPLE_CASE_TEST_HPP

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "framework/test_subscriber.hpp"
#include "yac_mocks.hpp"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;
using namespace std;

/**
 * Test provide use case for init yac object
 */
TEST_F(YacTest, YacWhenInit) {
  cout << "----------|Just init object|----------" << endl;

  MockYacNetwork network_;

  MockYacCryptoProvider crypto_;

  MockTimer timer_;

  auto fake_delay_ = 100500;

  auto order = ClusterOrdering::create(default_peers);
  ASSERT_TRUE(order);

  auto yac_ = Yac::create(YacVoteStorage(),
                          std::make_shared<MockYacNetwork>(network_),
                          std::make_shared<MockYacCryptoProvider>(crypto_),
                          std::make_shared<MockTimer>(timer_),
                          *order,
                          fake_delay_);

  network_.subscribe(yac_);
}

/**
 * Test provide scenario when yac vote for hash
 */
TEST_F(YacTest, YacWhenVoting) {
  cout << "----------|YacWhenAchieveOneVote|----------" << endl;

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(default_peers.size());

  YacHash my_hash("my_proposal_hash", "my_block_hash");

  auto order = ClusterOrdering::create(default_peers);
  ASSERT_TRUE(order);

  yac->vote(my_hash, *order);
}

/**
 * Test provide scenario when yac cold started and achieve one vote
 */
TEST_F(YacTest, YacWhenColdStartAndAchieveOneVote) {
  cout << "----------|Coldstart - receive one vote|----------" << endl;

  // verify that commit not emitted
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 0);
  wrapper.subscribe();

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>()))
      .Times(1)
      .WillRepeatedly(Return(true));

  YacHash received_hash("my_proposal", "my_block");
  auto peer = default_peers.at(0);
  // assume that our peer receive message
  network->notification->on_vote(crypto->getVote(received_hash));

  ASSERT_TRUE(wrapper.validate());
}

/**
 * Test provide scenario
 * when yac cold started and achieve supermajority of  votes
 */
TEST_F(YacTest, YacWhenColdStartAndAchieveSupermajorityOfVotes) {
  cout << "----------|Start => receive supermajority of votes"
          "|----------"
       << endl;

  // verify that commit not emitted
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 0);
  wrapper.subscribe();

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>()))
      .Times(default_peers.size())
      .WillRepeatedly(Return(true));

  YacHash received_hash("my_proposal", "my_block");
  for (size_t i = 0; i < default_peers.size(); ++i) {
    network->notification->on_vote(crypto->getVote(received_hash));
  }

  ASSERT_TRUE(wrapper.validate());
}

/**
 * Test provide scenario
 * when yac cold started and achieve commit
 */
TEST_F(YacTest, YacWhenColdStartAndAchieveCommitMessage) {
  cout << "----------|Start => receive commit|----------" << endl;
  YacHash propagated_hash("my_proposal", "my_block");

  // verify that commit emitted
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 1);
  wrapper.subscribe([propagated_hash](auto commit_hash) {
    ASSERT_EQ(propagated_hash, commit_hash.votes.at(0).hash);
  });

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).WillOnce(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).Times(0);

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  auto committed_peer = default_peers.at(0);
  auto msg = CommitMessage(std::vector<VoteMessage>{});
  for (size_t i = 0; i < default_peers.size(); ++i) {
    msg.votes.push_back(create_vote(propagated_hash, std::to_string(i)));
  }
  network->notification->on_commit(msg);

  ASSERT_TRUE(wrapper.validate());
}
#endif  // IROHA_YAC_SIMPLE_CASE_TEST_HPP
