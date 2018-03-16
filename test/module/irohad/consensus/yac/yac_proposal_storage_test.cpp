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

#include <gtest/gtest.h>

#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacProposalStorage");

class YacProposalStorageTest : public ::testing::Test {
 public:
  YacHash hash;
  uint64_t number_of_peers;
  YacProposalStorage storage = YacProposalStorage("proposal", 4);
  std::vector<VoteMessage> valid_votes;

  void SetUp() override {
    hash = YacHash("proposal", "commit");
    number_of_peers = 7;
    storage = YacProposalStorage(hash.proposal_hash, number_of_peers);
    valid_votes = [this]() {
      std::vector<VoteMessage> votes;
      for (auto i = 0u; i < number_of_peers; ++i) {
        votes.push_back(create_vote(hash, std::to_string(i)));
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
  auto other_hash = YacHash(hash.proposal_hash, "other_commit");
  for (auto i = 0; i < 2; ++i) {
    auto answer = storage.insert(
        create_vote(other_hash, std::to_string(valid_votes.size() + 1 + i)));
    ASSERT_EQ(boost::none, answer);
  }

  // insert more one for other hash
  auto answer = storage.insert(
      create_vote(other_hash, std::to_string(2 * valid_votes.size() + 1)));
  ASSERT_NE(boost::none, answer);
  ASSERT_EQ(6, boost::get<RejectMessage>(*answer).votes.size());
}
