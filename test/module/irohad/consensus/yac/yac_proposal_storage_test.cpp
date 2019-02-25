/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_proposal_storage.hpp"

#include <gtest/gtest.h>

#include "consensus/yac/storage/yac_common.hpp"
#include "logger/logger.hpp"

#include "module/irohad/consensus/yac/yac_test_util.hpp"

using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacProposalStorage");

class YacProposalStorageTest : public ::testing::Test {
 public:
  YacHash hash;
  PeersNumberType number_of_peers;
  YacProposalStorage storage =
      YacProposalStorage(iroha::consensus::Round{1, 1}, 4);
  std::vector<VoteMessage> valid_votes;

  void SetUp() override {
    hash = YacHash(iroha::consensus::Round{1, 1}, "proposal", "commit");
    number_of_peers = 7;
    storage =
        YacProposalStorage(iroha::consensus::Round{1, 1}, number_of_peers);
    valid_votes = [this]() {
      std::vector<VoteMessage> votes;
      for (auto i = 0u; i < number_of_peers; ++i) {
        votes.push_back(createVote(hash, std::to_string(i)));
      }
      return votes;
    }();
  }
};

TEST_F(YacProposalStorageTest, YacProposalStorageWhenCommitCase) {
  log_->info(
      "Init storage => insert unique votes => "
      "expected commit");

  for (auto i = 0u; i < 4; ++i) {
    ASSERT_EQ(boost::none, storage.insert(valid_votes.at(i)));
  }

  for (auto i = 4u; i < 7; ++i) {
    auto commit = storage.insert(valid_votes.at(i));
    log_->info("Commit: {}", logger::opt_to_string(commit, [](auto answer) {
                 return "value";
               }));
    ASSERT_NE(boost::none, commit);
    ASSERT_EQ(i + 1, boost::get<CommitMessage>(*commit).votes.size());
  }
}

TEST_F(YacProposalStorageTest, YacProposalStorageWhenInsertNotUnique) {
  log_->info(
      "Init storage => insert not-unique votes => "
      "expected absence of commit");

  for (auto i = 0; i < 7; ++i) {
    auto fixed_index = 0;
    ASSERT_EQ(boost::none, storage.insert(valid_votes.at(fixed_index)));
  }
}

TEST_F(YacProposalStorageTest, YacProposalStorageWhenRejectCase) {
  log_->info(
      "Init storage => insert votes for reject case => "
      "expected absence of commit");

  // insert 3 vote for hash 1
  for (auto i = 0; i < 3; ++i) {
    ASSERT_EQ(boost::none, storage.insert(valid_votes.at(i)));
  }

  // insert 2 for other hash
  auto other_hash = YacHash(iroha::consensus::Round{1, 1},
                            hash.vote_hashes.proposal_hash,
                            "other_commit");
  for (auto i = 0; i < 2; ++i) {
    auto answer = storage.insert(
        createVote(other_hash, std::to_string(valid_votes.size() + 1 + i)));
    ASSERT_EQ(boost::none, answer);
  }

  // insert more one for other hash
  auto answer = storage.insert(
      createVote(other_hash, std::to_string(2 * valid_votes.size() + 1)));
  ASSERT_NE(boost::none, answer);
  ASSERT_EQ(6, boost::get<RejectMessage>(*answer).votes.size());
}
