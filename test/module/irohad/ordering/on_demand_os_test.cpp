/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include "builders/protobuf/transaction.hpp"
#include "datetime/time.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;

class OnDemandOsTest : public ::testing::Test {
 public:
  std::shared_ptr<OnDemandOrderingService> os;
  const uint64_t transaction_limit = 20;
  const uint32_t proposal_limit = 5;
  const Round initial_round = {2, 1}, target_round = {4, 1},
              commit_round = {3, 1}, reject_round = {2, 2};

  void SetUp() override {
    os = std::make_shared<OnDemandOrderingServiceImpl>(
        transaction_limit, proposal_limit, initial_round);
  }

  /**
   * Generate transactions with provided range
   * @param os - ordering service for insertion
   * @param range - pair of [from, to)
   */
  void generateTransactionsAndInsert(Round round,
                                     std::pair<uint64_t, uint64_t> range) {
    auto now = iroha::time::now();
    OnDemandOrderingService::CollectionType collection;
    for (auto i = range.first; i < range.second; ++i) {
      collection.push_back(std::make_unique<shared_model::proto::Transaction>(
          shared_model::proto::TransactionBuilder()
              .createdTime(now + i)
              .creatorAccountId("foo@bar")
              .createAsset("asset", "domain", 1)
              .quorum(1)
              .build()
              .signAndAddSignature(
                  shared_model::crypto::DefaultCryptoAlgorithmType::
                      generateKeypair())
              .finish()));
    }
    os->onTransactions(round, std::move(collection));
  }
};

/**
 * @given initialized on-demand OS
 * @when  don't send transactions
 * AND initiate next round
 * @then  check that previous round doesn't have proposal
 */
TEST_F(OnDemandOsTest, EmptyRound) {
  ASSERT_FALSE(os->onRequestProposal(initial_round));

  os->onCollaborationOutcome(commit_round);

  ASSERT_FALSE(os->onRequestProposal(initial_round));
}

/**
 * @given initialized on-demand OS
 * @when  send number of transactions less that limit
 * AND initiate next round
 * @then  check that previous round has all transaction
 */
TEST_F(OnDemandOsTest, NormalRound) {
  generateTransactionsAndInsert(target_round, {1, 2});

  os->onCollaborationOutcome(commit_round);

  ASSERT_TRUE(os->onRequestProposal(target_round));
}

/**
 * @given initialized on-demand OS
 * @when  send number of transactions greater that limit
 * AND initiate next round
 * @then  check that previous round has only limit of transactions
 * AND the rest of transactions isn't appeared in next after next round
 */
TEST_F(OnDemandOsTest, OverflowRound) {
  generateTransactionsAndInsert(target_round, {1, transaction_limit * 2});

  os->onCollaborationOutcome(commit_round);

  ASSERT_TRUE(os->onRequestProposal(target_round));
  ASSERT_EQ(transaction_limit,
            (*os->onRequestProposal(target_round))->transactions().size());
}

/**
 * @given initialized on-demand OS
 * @when  send transactions from different threads
 * AND initiate next round
 * @then  check that all transactions are appeared in proposal
 */
TEST_F(OnDemandOsTest, DISABLED_ConcurrentInsert) {
  auto large_tx_limit = 10000u;
  os = std::make_shared<OnDemandOrderingServiceImpl>(
      large_tx_limit, proposal_limit, initial_round);

  auto call = [this](auto bounds) {
    for (auto i = bounds.first; i < bounds.second; ++i) {
      this->generateTransactionsAndInsert(target_round, {i, i + 1});
    }
  };

  std::thread one(call, std::make_pair(0u, large_tx_limit / 2));
  std::thread two(call, std::make_pair(large_tx_limit / 2, large_tx_limit));
  one.join();
  two.join();
  os->onCollaborationOutcome(commit_round);
  ASSERT_EQ(large_tx_limit,
            os->onRequestProposal(target_round).get()->transactions().size());
}

/**
 * @given initialized on-demand OS
 * @when  insert proposal_limit rounds twice
 * @then  on second rounds check that old proposals are expired
 */
TEST_F(OnDemandOsTest, Erase) {
  for (auto i = commit_round.block_round;
       i < commit_round.block_round + proposal_limit;
       ++i) {
    generateTransactionsAndInsert({i + 1, commit_round.reject_round}, {1, 2});
    os->onCollaborationOutcome({i, commit_round.reject_round});
    ASSERT_TRUE(os->onRequestProposal({i + 1, commit_round.reject_round}));
  }

  for (BlockRoundType i = commit_round.block_round + proposal_limit;
       i < commit_round.block_round + 2 * proposal_limit;
       ++i) {
    generateTransactionsAndInsert({i + 1, commit_round.reject_round}, {1, 2});
    os->onCollaborationOutcome({i, commit_round.reject_round});
    ASSERT_FALSE(os->onRequestProposal(
        {i + 1 - proposal_limit, commit_round.reject_round}));
  }
}

/**
 * @given initialized on-demand OS
 * @when  insert proposal_limit rounds twice
 * AND outcome is reject
 * @then  on second rounds check that old proposals are expired
 */
TEST_F(OnDemandOsTest, EraseReject) {
  for (auto i = reject_round.reject_round;
       i < reject_round.reject_round + proposal_limit;
       ++i) {
    generateTransactionsAndInsert({reject_round.block_round, i + 1}, {1, 2});
    os->onCollaborationOutcome({reject_round.block_round, i});
    ASSERT_TRUE(os->onRequestProposal({reject_round.block_round, i + 1}));
  }

  for (RejectRoundType i = reject_round.reject_round + proposal_limit;
       i < reject_round.reject_round + 2 * proposal_limit;
       ++i) {
    generateTransactionsAndInsert({reject_round.block_round, i + 1}, {1, 2});
    os->onCollaborationOutcome({reject_round.block_round, i});
    ASSERT_FALSE(os->onRequestProposal(
        {reject_round.block_round, i + 1 - proposal_limit}));
  }
}
