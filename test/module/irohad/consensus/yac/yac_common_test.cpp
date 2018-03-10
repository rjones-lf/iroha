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

#include "consensus/yac/storage/yac_common.hpp"
#include <gtest/gtest.h>
#include "consensus/consensus_common.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using namespace iroha::consensus;
using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacCommon");

TEST(YacCommonTest, SupermajorityFunctionForAllCases2) {
  log_->info("-----------| F(x, 2), x in {0..3} -----------");

  int N = 2;
  ASSERT_FALSE(hasSupermajority(0, N));
  ASSERT_FALSE(hasSupermajority(1, N));
  ASSERT_TRUE(hasSupermajority(2, N));
  ASSERT_FALSE(hasSupermajority(3, N));
}

TEST(YacCommonTest, SupermajorityFunctionForAllCases4) {
  log_->info("-----------| F(x, 4), x in {0..5} |-----------");

  int N = 4;
  ASSERT_FALSE(hasSupermajority(0, N));
  ASSERT_FALSE(hasSupermajority(1, N));
  ASSERT_FALSE(hasSupermajority(2, N));
  ASSERT_TRUE(hasSupermajority(3, N));
  ASSERT_TRUE(hasSupermajority(4, N));
  ASSERT_FALSE(hasSupermajority(5, N));
}

TEST(YacCommonTest, RejectProofSuccessfulCase) {
  log_->info("-----------| RejectProof(x, 6, 7) in {1..3} |-----------");

  ASSERT_TRUE(hasReject(1, 6, 7));
  ASSERT_TRUE(hasReject(2, 6, 7));
  ASSERT_TRUE(hasReject(3, 6, 7));
}

TEST(YacCommonTest, RejectProofNegativeCase) {
  log_->info("-----------| RejectProof(x, 6, 7) in {4..6}|-----------");

  ASSERT_FALSE(hasReject(4, 6, 7));
  ASSERT_FALSE(hasReject(5, 6, 7));
  ASSERT_FALSE(hasReject(6, 6, 7));
}

TEST(YacCommonTest, SameProposalTest) {
  log_->info("-----------| Verify ok and fail cases |-----------");

  YacHash hash("proposal", "commit");
  std::vector<VoteMessage> votes{create_vote(hash, "two"),
                                 create_vote(hash, "three"),
                                 create_vote(hash, "four")};

  ASSERT_TRUE(sameProposals(votes));

  votes.push_back(create_vote(YacHash("not-proposal", "commit"), "five"));
  ASSERT_FALSE(sameProposals(votes));
}

TEST(YacCommonTest, getProposalHashTest) {
  log_->info("-----------| Verify ok and fail cases |-----------");

  YacHash hash("proposal", "commit");
  std::vector<VoteMessage> votes{create_vote(hash, "two"),
                                 create_vote(hash, "three"),
                                 create_vote(hash, "four")};

  ASSERT_EQ(hash.proposal_hash, getProposalHash(votes).value());

  votes.push_back(create_vote(YacHash("not-proposal", "commit"), "five"));
  ASSERT_EQ(nonstd::nullopt, getProposalHash(votes));
}
