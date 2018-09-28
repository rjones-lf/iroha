/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_common.hpp"

#include <gtest/gtest.h>
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"

using namespace iroha::consensus;
using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacCommon");

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
  ASSERT_FALSE(getProposalHash(votes));
}
