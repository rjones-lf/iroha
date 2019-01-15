/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */


#include <gtest/gtest.h>

#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger.hpp"

#include "module/irohad/consensus/yac/yac_test_util.hpp"

using namespace iroha::consensus;
using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacCommon");

TEST(YacCommonTest, SameProposalTest) {
  log_->info("-----------| Verify ok and fail cases |-----------");

  YacHash hash(Round{1, 1}, "proposal", "commit");
  std::vector<VoteMessage> votes{createVote(hash, "two"),
                                 createVote(hash, "three"),
                                 createVote(hash, "four")};

  ASSERT_TRUE(sameKeys(votes));

  votes.push_back(
      createVote(YacHash(Round{1, 2}, "not-proposal", "commit"), "five"));
  ASSERT_FALSE(sameKeys(votes));
}

TEST(YacCommonTest, getProposalHashTest) {
  log_->info("-----------| Verify ok and fail cases |-----------");

  YacHash hash(Round{1, 1}, "proposal", "commit");
  std::vector<VoteMessage> votes{createVote(hash, "two"),
                                 createVote(hash, "three"),
                                 createVote(hash, "four")};

  ASSERT_EQ(hash.vote_round, getKey(votes).value());

  votes.push_back(
      createVote(YacHash(Round{1, 2}, "not-proposal", "commit"), "five"));
  ASSERT_FALSE(getKey(votes));
}
