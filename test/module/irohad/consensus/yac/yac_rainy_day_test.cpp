/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "framework/test_subscriber.hpp"

#include "module/irohad/consensus/yac/yac_fixture.hpp"

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
  auto my_peers = decltype(default_peers)(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  initYac(my_order.value());

  EXPECT_CALL(*network, sendState(_, _)).Times(2 * my_peers.size());

  EXPECT_CALL(*timer, deny()).Times(0);

  EXPECT_CALL(*crypto, verify(_)).WillRepeatedly(Return(true));

  YacHash hash1(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash");
  YacHash hash2(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash2");
  yac->vote(hash1, my_order.value());

  for (auto i = 0; i < 2; ++i) {
    yac->onState({createVote(hash1, std::to_string(i))});
  };
  for (auto i = 2; i < 4; ++i) {
    yac->onState({createVote(hash2, std::to_string(i))});
  };
}

/**
 * @given yac consensus
 * @when 2 peers vote for one hash and 2 for another, but yac_crypto verify
 * always returns false
 * @then reject is not propagated
 */
TEST_F(YacTest, InvalidCaseWhenDoesNotVerify) {
  auto my_peers = decltype(default_peers)(
      {default_peers.begin(), default_peers.begin() + 4});
  ASSERT_EQ(4, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  initYac(my_order.value());

  EXPECT_CALL(*network, sendState(_, _)).Times(0);

  EXPECT_CALL(*timer, deny()).Times(0);

  EXPECT_CALL(*crypto, verify(_)).WillRepeatedly(Return(false));

  YacHash hash1(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash");
  YacHash hash2(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash2");

  for (auto i = 0; i < 2; ++i) {
    yac->onState({createVote(hash1, std::to_string(i))});
  };
  for (auto i = 2; i < 4; ++i) {
    yac->onState({createVote(hash2, std::to_string(i))});
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
  auto my_peers = decltype(default_peers)(
      {default_peers.begin(), default_peers.begin() + peers_number});
  ASSERT_EQ(peers_number, my_peers.size());

  auto my_order = ClusterOrdering::create(my_peers);
  ASSERT_TRUE(my_order);

  initYac(my_order.value());

  EXPECT_CALL(*network, sendState(_, _))
      .Times(my_peers.size() + 1);  // $(peers.size()) sendings done during
                                    // multicast + 1 for single peer, who votes
                                    // after reject happened

  EXPECT_CALL(*timer, deny()).Times(1);

  EXPECT_CALL(*crypto, verify(_)).WillRepeatedly(Return(true));

  YacHash hash1(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash");
  YacHash hash2(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash2");

  std::vector<VoteMessage> votes;
  for (size_t i = 0; i < peers_number / 2; ++i) {
    auto peer = my_order->getPeers().at(i);
    auto pubkey = shared_model::crypto::toBinaryString(peer->pubkey());
    votes.push_back(createVote(hash1, pubkey));
  };
  for (size_t i = peers_number / 2; i < peers_number - 1; ++i) {
    auto peer = my_order->getPeers().at(i);
    auto pubkey = shared_model::crypto::toBinaryString(peer->pubkey());
    votes.push_back(createVote(hash2, pubkey));
  };

  for (const auto &vote : votes) {
    yac->onState({vote});
  }

  yac->onState(votes);
  auto peer = my_order->getPeers().back();
  auto pubkey = shared_model::crypto::toBinaryString(peer->pubkey());
  yac->onState({createVote(hash1, pubkey)});
}
