/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "consensus/yac/storage/yac_proposal_storage.hpp"

#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_fixture.hpp"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;
using namespace std;

/**
 * Test provide scenario when yac vote for hash
 */
TEST_F(YacTest, YacWhenVoting) {
  cout << "----------|YacWhenAchieveOneVote|----------" << endl;

  EXPECT_CALL(*network, sendState(_, _)).Times(default_peers.size());

  YacHash my_hash(
      iroha::consensus::Round{1, 1}, "my_proposal_hash", "my_block_hash");

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
  auto wrapper = make_test_subscriber<CallExact>(yac->onOutcome(), 0);
  wrapper.subscribe();

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(_)).Times(1).WillRepeatedly(Return(true));

  YacHash received_hash(
      iroha::consensus::Round{1, 1}, "my_proposal", "my_block");
  auto peer = default_peers.at(0);
  // assume that our peer receive message
  network->notification->onState({crypto->getVote(received_hash)});

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
  auto wrapper = make_test_subscriber<CallExact>(yac->onOutcome(), 0);
  wrapper.subscribe();

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(_))
      .Times(default_peers.size())
      .WillRepeatedly(Return(true));

  YacHash received_hash(
      iroha::consensus::Round{1, 1}, "my_proposal", "my_block");
  for (size_t i = 0; i < default_peers.size(); ++i) {
    network->notification->onState({crypto->getVote(received_hash)});
  }

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized YAC with empty storage
 * @when receive commit message
 * @then commit is not broadcasted
 * AND commit is emitted to observable
 */
TEST_F(YacTest, YacWhenColdStartAndAchieveCommitMessage) {
  YacHash propagated_hash(
      iroha::consensus::Round{1, 1}, "my_proposal", "my_block");

  // verify that commit emitted
  auto wrapper = make_test_subscriber<CallExact>(yac->onOutcome(), 1);
  wrapper.subscribe([propagated_hash](auto commit_hash) {
    ASSERT_EQ(propagated_hash,
              boost::get<CommitMessage>(commit_hash).votes.at(0).hash);
  });

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(_)).WillOnce(Return(true));

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  auto committed_peer = default_peers.at(0);
  auto msg = CommitMessage(std::vector<VoteMessage>{});
  for (size_t i = 0; i < default_peers.size(); ++i) {
    msg.votes.push_back(createVote(propagated_hash, std::to_string(i)));
  }
  network->notification->onState(msg.votes);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized YAC
 * @when receive supermajority of votes for a hash
 * @then commit is sent to the network before notifying subscribers
 */
TEST_F(YacTest, PropagateCommitBeforeNotifyingSubscribersApplyVote) {
  EXPECT_CALL(*crypto, verify(_))
      .Times(default_peers.size())
      .WillRepeatedly(Return(true));
  std::vector<std::vector<VoteMessage>> messages;
  EXPECT_CALL(*network, sendState(_, _))
      .Times(default_peers.size())
      .WillRepeatedly(Invoke(
          [&](const auto &, const auto &msg) { messages.push_back(msg); }));

  yac->onOutcome().subscribe([&](auto msg) {
    // verify that commits are already sent to the network
    ASSERT_EQ(default_peers.size(), messages.size());
    messages.push_back(boost::get<CommitMessage>(msg).votes);
  });

  for (size_t i = 0; i < default_peers.size(); ++i) {
    yac->onState({createVote(YacHash{}, std::to_string(i))});
  }

  // verify that on_commit subscribers are notified
  ASSERT_EQ(default_peers.size() + 1, messages.size());
}

/**
 * @given initialized YAC
 * @when receive 2 * f votes for one hash
 * AND receive reject message which triggers commit
 * @then commit is NOT propagated in the network
 * AND it is passed to pipeline
 */
TEST_F(YacTest, PropagateCommitBeforeNotifyingSubscribersApplyReject) {
  EXPECT_CALL(*crypto, verify(_)).WillRepeatedly(Return(true));
  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));
  std::vector<std::vector<VoteMessage>> messages;
  EXPECT_CALL(*network, sendState(_, _))
      .Times(0)
      .WillRepeatedly(Invoke(
          [&](const auto &, const auto &msg) { messages.push_back(msg); }));

  yac->onOutcome().subscribe([&](auto msg) {
    // verify that commits are already sent to the network
    ASSERT_EQ(0, messages.size());
    messages.push_back(boost::get<CommitMessage>(msg).votes);
  });

  std::vector<VoteMessage> commit;

  auto f = (default_peers.size() - 1) / 3;
  for (size_t i = 0; i < 2 * f; ++i) {
    auto vote = createVote(YacHash{}, std::to_string(i));
    yac->onState({vote});
    commit.push_back(vote);
  }

  auto vote = createVote(YacHash{}, std::to_string(2 * f + 1));
  RejectMessage reject(
      {vote,
       createVote(YacHash(iroha::consensus::Round{1, 1}, "", "my_block"),
                  std::to_string(2 * f + 2))});
  commit.push_back(vote);

  yac->onState(reject.votes);
  yac->onState(commit);

  // verify that on_commit subscribers are notified
  ASSERT_EQ(1, messages.size());
}
