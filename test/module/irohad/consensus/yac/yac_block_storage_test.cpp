/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_block_storage.hpp"

#include <gtest/gtest.h>

#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger.hpp"

#include "module/irohad/consensus/yac/yac_test_util.hpp"

using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacBlockStorage");

class YacBlockStorageTest : public ::testing::Test {
 public:
  YacHash hash;
  PeersNumberType number_of_peers;
  YacBlockStorage storage = YacBlockStorage(
      YacHash(iroha::consensus::Round{1, 1}, "proposal", "commit"), 4);
  std::vector<VoteMessage> valid_votes;

  void SetUp() override {
    hash = YacHash(iroha::consensus::Round{1, 1}, "proposal", "commit");
    number_of_peers = 4;
    storage = YacBlockStorage(hash, number_of_peers);
    valid_votes = {createVote(hash, "one"),
                   createVote(hash, "two"),
                   createVote(hash, "three"),
                   createVote(hash, "four")};
  }
};

TEST_F(YacBlockStorageTest, YacBlockStorageWhenNormalDataInput) {
  log_->info("-----------| Sequentially insertion of votes |-----------");

  auto insert_1 = storage.insert(valid_votes.at(0));
  ASSERT_EQ(boost::none, insert_1);

  auto insert_2 = storage.insert(valid_votes.at(1));
  ASSERT_EQ(boost::none, insert_2);

  auto insert_3 = storage.insert(valid_votes.at(2));
  ASSERT_NE(boost::none, insert_3);
  ASSERT_EQ(3, boost::get<CommitMessage>(*insert_3).votes.size());

  auto insert_4 = storage.insert(valid_votes.at(3));
  ASSERT_NE(boost::none, insert_4);
  ASSERT_EQ(4, boost::get<CommitMessage>(*insert_4).votes.size());
}

TEST_F(YacBlockStorageTest, YacBlockStorageWhenNotCommittedAndCommitAcheive) {
  log_->info("-----------| Insert vote => insert commit |-----------");

  auto insert_1 = storage.insert(valid_votes.at(0));
  ASSERT_EQ(boost::none, insert_1);

  decltype(YacBlockStorageTest::valid_votes) for_insert(valid_votes.begin() + 1,
                                                        valid_votes.end());
  auto insert_commit = storage.insert(for_insert);
  ASSERT_EQ(4, boost::get<CommitMessage>(*insert_commit).votes.size());
}

TEST_F(YacBlockStorageTest, YacBlockStorageWhenGetVotes) {
  log_->info("-----------| Init storage => verify internal votes |-----------");

  storage.insert(valid_votes);
  ASSERT_EQ(valid_votes, storage.getVotes());
}

TEST_F(YacBlockStorageTest, YacBlockStorageWhenIsContains) {
  log_->info(
      "-----------| Init storage => "
      "verify ok and fail cases of contains |-----------");

  decltype(YacBlockStorageTest::valid_votes) for_insert(
      valid_votes.begin(), valid_votes.begin() + 2);

  storage.insert(for_insert);

  ASSERT_TRUE(storage.isContains(valid_votes.at(0)));
  ASSERT_FALSE(storage.isContains(valid_votes.at(3)));
}
