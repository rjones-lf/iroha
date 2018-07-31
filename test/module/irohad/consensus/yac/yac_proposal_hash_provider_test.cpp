/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_proposal_hash_provider_impl.hpp"

#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::network;
using namespace iroha::consensus::yac;

using ::testing::ReturnRef;

struct YacProposalHashProviderTest : public ::testing::Test {
  void SetUp() override {
    proposal = std::make_unique<MockProposal>();
    EXPECT_CALL(*proposal, blob()).WillRepeatedly(ReturnRef(blob));
    hash = proposal->hash();
  }

  YacProposalHashProviderImpl hash_provider;
  std::unique_ptr<MockProposal> proposal;
  shared_model::interface::types::BlobType blob;
  shared_model::interface::types::HashType hash;
  iroha::ordering::transport::RoundType round{1, 1};
};

/**
 * @given non-empty proposal optional and round number
 * @when hash is made from proposal and round
 * AND proposal info is made from hash
 * @then hash and round from info matches given data
 */
TEST_F(YacProposalHashProviderTest, YacHashFromProposal) {
  std::unique_ptr<shared_model::interface::Proposal> iproposal(
      std::move(proposal));
  auto yac_hash =
      hash_provider.makeHash(ProposalVote{std::move(iproposal), round});

  auto model_hash = hash_provider.makeProposalInfo(yac_hash);

  ASSERT_TRUE(model_hash.hash);
  ASSERT_EQ(hash, model_hash.hash.value());
  ASSERT_EQ(round, model_hash.round);
}

/**
 * @given empty proposal optional and round number
 * @when hash is made from proposal and round
 * AND proposal info is made from hash
 * @then empty hash and round from info matches given data
 */
TEST_F(YacProposalHashProviderTest, YacHashFromNone) {
  auto yac_hash = hash_provider.makeHash(ProposalVote{boost::none, round});

  auto model_hash = hash_provider.makeProposalInfo(yac_hash);

  ASSERT_FALSE(model_hash.hash);
  ASSERT_EQ(round, model_hash.round);
}
