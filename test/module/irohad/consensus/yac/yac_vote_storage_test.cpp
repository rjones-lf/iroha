/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_vote_storage.hpp"

#include <gtest/gtest.h>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include "consensus/yac/storage/yac_common.hpp"
#include "logger/logger.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "utils/string_builder.hpp"

using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacProposalStorage");

class YacVoteStorageTest : public ::testing::Test {
 public:
  iroha::consensus::Round round{1, 1};
  YacHash hash = YacHash(round, "proposal", "commit");
  PeersNumberType number_of_peers = 7;
  YacVoteStorage storage;
  std::vector<VoteMessage> valid_votes;

  void SetUp() override {
    std::generate_n(std::back_inserter(valid_votes), number_of_peers, [this] {
      return create_vote(this->hash, std::to_string(this->valid_votes.size()));
    });
  }
};

/**
 * @given a commit was done
 * @when YacProposalStorage::getLastCommit() called
 * @then the commit message for that commit is returned
 * @and it contains all the votes for that commit
 */
TEST_F(YacVoteStorageTest, GetLastCommitSuccessfulTest) {
  // create a commit
  size_t num_stored = 0;
  while (num_stored < number_of_peers) {
    const auto store_result =
        storage.store({valid_votes.at(num_stored++)}, number_of_peers);
    if (store_result) {
      ASSERT_NO_THROW(boost::get<CommitMessage>(*store_result))
          << "Got something else than a commit!";
      break;
    }
  }

  // switch the state
  storage.nextProcessingState(round);
  storage.nextProcessingState(round);
  ASSERT_EQ(storage.getProcessingState(round), ProposalState::kSentProcessed)
      << "Round not in the processed state!";

  // ask the storage the last commit
  const auto commit_message = storage.getLastCommit();
  ASSERT_TRUE(commit_message) << "Did not get the last commit!";

  const auto stored_votes =
      valid_votes | boost::adaptors::sliced(0, num_stored);

  const auto votes_to_string = [](const auto &votes) {
    return shared_model::detail::PrettyStringBuilder()
        .init("votes")
        .appendAll(votes, [](auto vote) { return vote.toString(); })
        .finalize();
  };

  std::vector<VoteMessage> missing_votes;
  boost::range::set_difference(
      stored_votes, commit_message->votes, std::back_inserter(missing_votes));
  EXPECT_EQ(boost::size(missing_votes), 0)
      << "Commit message is missing the following votes: "
      << votes_to_string(missing_votes) << ".";

  std::vector<VoteMessage> extra_votes;
  boost::range::set_difference(
      commit_message->votes, stored_votes, std::back_inserter(extra_votes));
  EXPECT_EQ(boost::size(extra_votes), 0)
      << "Commit message contains the following extra votes: "
      << votes_to_string(extra_votes) << ".";
}
