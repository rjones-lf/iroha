/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <memory>

#include "builders/protobuf/transaction.hpp"
#include "datetime/time.hpp"
#include "ordering/impl/on_demand_ordering_service_impl.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;

class OnDemandOsTest : public ::testing::Test {
 public:
  std::shared_ptr<OnDemandOrderingService> os;
  const uint64_t transaction_limit = 2;
  const uint64_t proposal_limit = 30;
  const RoundType target_round = RoundType(1, 1);

  void SetUp() override {
    os = std::make_shared<OnDemandOrderingServiceImpl>(
        transaction_limit, proposal_limit, std::make_pair(2, 1));
  }

  /**
   * Generate transactions with provided range
   * @param os - ordering service for insertion
   * @param range - pair of [from, to)
   */
  static void generateTransactionsAndInsert(
      OnDemandOrderingService &os, std::pair<uint64_t, uint64_t> range) {
    OnDemandOrderingService::CollectionType collection;
    for (auto i = range.first; i < range.second; ++i) {
      auto tx = shared_model::proto::TransactionBuilder()
                    .createdTime(iroha::time::now())
                    .creatorAccountId("admin@ru")
                    .addAssetQuantity("admin@tu", "coin#coin", "1.0")
                    .quorum(1)
                    .build()
                    .signAndAddSignature(
                        shared_model::crypto::DefaultCryptoAlgorithmType::
                            generateKeypair())
                    .finish();
      collection.push_back(
          std::make_shared<shared_model::proto::Transaction>(tx));
    }
    os.onTransactions(collection);
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

  os->onCollaborationOutcome(RoundOutput::SUCCESSFUL, target_round);

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

  os->onCollaborationOutcome(RoundOutput::SUCCESSFUL, target_round);

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
  generateTransactionsAndInsert(*os, {1, proposal_limit * 2});

  os->onCollaborationOutcome(RoundOutput::SUCCESSFUL, target_round);

  ASSERT_TRUE(os->onRequestProposal(target_round));
  ASSERT_TRUE((*os->onRequestProposal(target_round))->transactions().size()
              == proposal_limit);
}

/**
 * @given initialized on-demand OS
 * @when  send transactions from different threads
 * AND initiate next round
 * @then  check that all transactions are appeared in proposal
 */
TEST_F(OnDemandOsTest, ConcurrentInsert) {}

/**
 * @given initialized on-demand OS
 * @when  initiate rounds with successful and reject
 * @then  check that old
 */
TEST_F(OnDemandOsTest, Erase) {
  auto round = RoundType(1, 1);
  for (auto i = 1u; i < proposal_limit + 1; ++i) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    os->onCollaborationOutcome(RoundOutput::SUCCESSFUL, round);
    round = {i, round.second};
    ASSERT_TRUE(os->onRequestProposal({i, 1}));
  }

  for (uint64_t i = proposal_limit, j = 1; i < 2 * proposal_limit; ++i, ++j) {
    generateTransactionsAndInsert(*os, {1, proposal_limit});
    ASSERT_FALSE(os->onRequestProposal({i, 1}));
    os->onCollaborationOutcome(RoundOutput::SUCCESSFUL, round);
    round = {round.first + 1, round.second};
  }
}
