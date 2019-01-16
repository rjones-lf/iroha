/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

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
 * @given initialized yac
 * @when receive vote from unknown peer
 * @then commit not emitted
 */
TEST_F(YacTest, UnknownVoteBeforeCommit) {
  // verify that commit not emitted
  auto wrapper = make_test_subscriber<CallExact>(yac->onOutcome(), 0);
  wrapper.subscribe();

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*crypto, verify(_)).Times(1).WillRepeatedly(Return(true));

  VoteMessage vote;
  vote.hash = YacHash(iroha::consensus::Round{1, 1}, "my_proposal", "my_block");
  std::string unknown = "unknown";
  vote.signature = createSig(unknown);

  // assume that our peer receive message
  network->notification->onState({vote});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized yac
 * AND received commit
 * @when receive vote from unknown peer for committed hash
 * @then commit not emitted
 */
TEST_F(YacTest, UnknownVoteAfterCommit) {
  auto my_peers = decltype(default_peers)(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  initYac(my_order.value());

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  EXPECT_CALL(*crypto, verify(_)).Times(2).WillRepeatedly(Return(true));

  YacHash my_hash(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash");

  std::vector<VoteMessage> votes;

  for (auto i = 0; i < 3; ++i) {
    votes.push_back(createVote(my_hash, std::to_string(i)));
  };
  yac->onState(votes);

  VoteMessage vote;
  vote.hash = my_hash;
  std::string unknown = "unknown";
  vote.signature = createSig(unknown);
  yac->onState({vote});
}
