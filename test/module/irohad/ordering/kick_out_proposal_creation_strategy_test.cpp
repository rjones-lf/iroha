/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/kick_out_proposal_creation_strategy.hpp"

#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "module/irohad/consensus/yac/mock_yac_supermajority_checker.hpp"

using namespace iroha::ordering;

using testing::_;
using testing::Return;

class KickOutProposalCreationStrategyTest : public testing::Test {
 public:
  void SetUp() override {
    for (auto i = 0u; i < number_of_peers; ++i) {
      peers.emplace_back(std::to_string(i));
    }

    supermajority_checker_ =
        std::make_shared<iroha::consensus::yac::MockSupermajorityChecker>();
    strategy_ = std::make_shared<KickOutProposalCreationStrategy>(
        supermajority_checker_);
  }

  std::shared_ptr<KickOutProposalCreationStrategy> strategy_;
  std::shared_ptr<iroha::consensus::yac::MockSupermajorityChecker>
      supermajority_checker_;

  std::vector<KickOutProposalCreationStrategy::PeerType> peers;
  size_t number_of_peers = 7;
  size_t f = 2;
};

/**
 * @given initialized kickOutStrategy
 *        @and onCollaborationOutcome is invoked for the first round
 * @when  onProposal calls F times with different peers for further rounds
 * @then  shouldCreateRound returns true
 */
TEST_F(KickOutProposalCreationStrategyTest, OnNonMaliciousCase) {
  EXPECT_CALL(*supermajority_checker_, hasMajority(0, number_of_peers))
      .WillOnce(Return(false));

  strategy_->onCollaborationOutcome(peers);

  ASSERT_EQ(true, strategy_->shouldCreateRound({1, 0}));

  for (auto i = 0u; i < f; ++i) {
    strategy_->onProposal(peers.at(i), {2, 0});
  }

  EXPECT_CALL(*supermajority_checker_, hasMajority(f, number_of_peers))
      .WillOnce(Return(false));
  ASSERT_EQ(true, strategy_->shouldCreateRound({2, 0}));
}

/**
 * @given initialized kickOutStrategy
 *        @and onCollaborationOutcome is invoked for the first round
 * @when  onProposal calls F + 1 times with different peers for further rounds
 * @then  onCollaborationOutcome returns false
 */
TEST_F(KickOutProposalCreationStrategyTest, OnMaliciousCase) {
  strategy_->onCollaborationOutcome(peers);

  auto requested = f + 1;
  for (auto i = 0u; i < requested; ++i) {
    strategy_->onProposal(peers.at(i), {2, 0});
  }

  EXPECT_CALL(*supermajority_checker_, hasMajority(requested, number_of_peers))
      .WillOnce(Return(true));
  ASSERT_EQ(false, strategy_->shouldCreateRound({2, 0}));
}

/**
 * @given initialized kickOutStrategy
 *        @and onCollaborationOutcome is invoked for the first round
 * @when  onProposal calls F + 1 times with one peer
 * @then  onCollaborationOutcome call returns true
 */
TEST_F(KickOutProposalCreationStrategyTest, RepeadedRequest) {
  strategy_->onCollaborationOutcome(peers);

  auto requested = f + 1;
  for (auto i = 0u; i < requested; ++i) {
    strategy_->onProposal(peers.at(0), {2, 0});
  }
  EXPECT_CALL(*supermajority_checker_, hasMajority(1, number_of_peers))
      .WillOnce(Return(false));
  ASSERT_EQ(true, strategy_->shouldCreateRound({2, 0}));
}

/**
 * @given initialized kickOutStrategy
 *        @and onCollaborationOutcome is invoked for the first round
 * @when  onProposal calls F times different peers
 *        @and 1 time with unknown peer
 * @then  onCollaborationOutcome call returns true
 */
TEST_F(KickOutProposalCreationStrategyTest, UnknownPeerRequestsProposal) {
  strategy_->onCollaborationOutcome(peers);

  for (auto i = 0u; i < f; ++i) {
    strategy_->onProposal(peers.at(i), {2, 0});
  }
  strategy_->onProposal(shared_model::crypto::PublicKey{"unknown"}, {2, 0});
  EXPECT_CALL(*supermajority_checker_, hasMajority(f, number_of_peers))
      .WillOnce(Return(false));
  ASSERT_EQ(true, strategy_->shouldCreateRound({2, 0}));
}
