/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_proposal_storage.hpp"

#include <gtest/gtest.h>

#include "consensus/yac/storage/yac_common.hpp"
#include "logger/logger.hpp"

#include "framework/test_logger.hpp"
#include "logger/logger_manager.hpp"
#include "module/irohad/consensus/yac/yac_test_util.hpp"

using namespace iroha::consensus::yac;

static logger::LoggerPtr log_ = getTestLogger("YacProposalStorage");

class YacProposalStorageTest : public ::testing::Test {
 public:
  YacHash hash{iroha::consensus::Round{1, 1}, "proposal", "commit"};
  PeersNumberType number_of_peers{7};
  YacProposalStorage storage{
      iroha::consensus::Round{1, 1},
      number_of_peers,
      getTestLoggerManager()->getChild("YacProposalStorage")};
  std::vector<VoteMessage> valid_votes;

  void SetUp() override {
    valid_votes.reserve(number_of_peers);
    std::generate_n(std::back_inserter(valid_votes), number_of_peers, [this] {
      static size_t counter = 0;
      return createVote(this->hash, std::to_string(counter++));
    });
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
