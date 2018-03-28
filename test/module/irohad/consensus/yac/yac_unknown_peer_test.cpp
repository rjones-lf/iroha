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

  VoteMessage vote;
  vote.hash = YacHash("my_proposal", "my_block");
  std::string unknown = "unknown";
  std::copy(unknown.begin(), unknown.end(), vote.signature.pubkey.begin());
  // assume that our peer receive message
  network->notification->on_vote(vote);

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

  // delay preference
  uint64_t wait_seconds = 10;
  delay = wait_seconds * 1000;

  yac = Yac::create(
      YacVoteStorage(), network, crypto, timer, my_order.value(), delay);

  EXPECT_CALL(*network, send_commit(_, _)).Times(0);
  EXPECT_CALL(*network, send_reject(_, _)).Times(0);
  EXPECT_CALL(*network, send_vote(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(AtLeast(1));

  EXPECT_CALL(*crypto, verify(An<CommitMessage>())).WillOnce(Return(true));
  EXPECT_CALL(*crypto, verify(An<RejectMessage>())).Times(0);
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillOnce(Return(true));

  YacHash my_hash("proposal_hash", "block_hash");

  std::vector<VoteMessage> votes;

  for (auto i = 0; i < 3; ++i) {
    votes.push_back(create_vote(my_hash, std::to_string(i)));
  };
  yac->on_commit(CommitMessage(votes));

  VoteMessage vote;
  vote.hash = my_hash;
  std::string unknown = "unknown";
  std::copy(unknown.begin(), unknown.end(), vote.signature.pubkey.begin());

  yac->on_vote(vote);
}
