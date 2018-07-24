/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builders/protobuf/transaction.hpp"
#include "datetime/time.hpp"
#include "ordering/impl/on_demand_ordering_service_impl.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;

class OnDemandOsTest : public ::testing::Test {
 public:
  std::shared_ptr<OnDemandOrderingService> os;
  const uint64_t transaction_limit = 20;
  const uint64_t proposal_limit = 5;
  const RoundType target_round = RoundType(2, 1);

  void SetUp() override {
    os = std::make_shared<OnDemandOrderingServiceImpl>(
        transaction_limit, proposal_limit, target_round);
  }

  /**
   * Generate transactions with provided range
   * @param os - ordering service for insertion
   * @param range - pair of [from, to)
   */
  static void generateTransactionsAndInsert(
      OnDemandOrderingService &os, std::pair<uint64_t, uint64_t> range) {
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
    os.onTransactions(std::move(collection));
  }
};

/**
 * @given initialized on-demand OS
 * @when  don't send transactions
 * AND initiate next round
 * @then  check that previous round doesn't have proposal
 */
TEST_F(OnDemandOsTest, EmptyRound) {
  auto target_round = RoundType(1, 1);
  ASSERT_FALSE(os->onRequestProposal(target_round));

  os->onCollaborationOutcome(RoundOutput::kCommitBlock);

  ASSERT_FALSE(os->onRequestProposal(target_round));
}

/**
 * @given initialized on-demand OS
 * @when  send number of transactions less that limit
 * AND initiate next round
 * @then  check that previous round has all transaction
 */
TEST_F(OnDemandOsTest, NormalRound) {
  generateTransactionsAndInsert(*os, {1, 2});

  os->onCollaborationOutcome(RoundOutput::kCommitBlock);

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
  generateTransactionsAndInsert(*os, {1, transaction_limit * 2});

  os->onCollaborationOutcome(RoundOutput::kCommitBlock);

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
  auto concurrent_os = std::make_shared<OnDemandOrderingServiceImpl>(
      large_tx_limit, proposal_limit, target_round);

  auto call = [&concurrent_os](auto bounds) {
    for (auto i = bounds.first; i < bounds.second; ++i) {
      generateTransactionsAndInsert(*concurrent_os, std::make_pair(i, i + 1));
    }
  };

  std::thread one(call, std::make_pair(0u, large_tx_limit / 2));
  std::thread two(call, std::make_pair(large_tx_limit / 2, large_tx_limit));
  one.join();
  two.join();
  concurrent_os->onCollaborationOutcome(RoundOutput::kCommitBlock);
  ASSERT_EQ(large_tx_limit,
            concurrent_os->onRequestProposal(target_round)
                .get()
                ->transactions()
                .size());
}

/**
 * @given initialized on-demand OS
 * @when  insert proposal_limit rounds twice
 * @then  on second rounds check that old proposals are expired
 */
TEST_F(OnDemandOsTest, Erase) {
  auto round = target_round;
  for (auto i = target_round.first; i < proposal_limit + 1; ++i) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    os->onCollaborationOutcome(RoundOutput::kCommitBlock);
    round = {i, round.second};
    ASSERT_TRUE(os->onRequestProposal({i, 1}));
  }

  for (uint64_t i = proposal_limit + 1, j = 1; i < 2 * proposal_limit;
       ++i, ++j) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    ASSERT_FALSE(os->onRequestProposal({i, 1}));
    os->onCollaborationOutcome(RoundOutput::kCommitBlock);
    round = {round.first + 1, round.second};
  }
}

/**
 * @given initialized on-demand OS
 * @when  insert proposal_limit rounds twice
 * AND outcome is reject
 * @then  on second rounds check that old proposals are expired
 */
TEST_F(OnDemandOsTest, EraseReject) {
  auto round = target_round;
  for (auto i = target_round.second; i < proposal_limit + 1; ++i) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    os->onCollaborationOutcome(RoundOutput::kReject);
    round = {round.first, i};
    ASSERT_TRUE(os->onRequestProposal({round.first, i}));
  }

  for (uint64_t i = proposal_limit + 1, j = 1; i < 2 * proposal_limit;
       ++i, ++j) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    ASSERT_FALSE(os->onRequestProposal({round.first, i}));
    os->onCollaborationOutcome(RoundOutput::kReject);
    round = {round.first, round.second};
  }
}
