/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
