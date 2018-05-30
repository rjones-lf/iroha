/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

class AddAssetQuantity : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_add_asset_qty}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  const std::string kAmount = "1.0";
};

/**
 * @given some user with can_add_asset_qty permission
 * @when execute tx with AddAssetQuantity command
 * @then there is the tx in proposal
 */
TEST_F(AddAssetQuantity, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(kUserId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user without can_add_asset_qty permission
 * @when execute tx with AddAssetQuantity command
 * @then there is no tx in proposal
 */
TEST_F(AddAssetQuantity, NoPermissions) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(kUserId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with AddAssetQuantity command with negative amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(AddAssetQuantity, NegativeAmount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(kUserId, kAsset, "-1.0")),
              checkStatelessInvalid);
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with AddAssetQuantity command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(AddAssetQuantity, ZeroAmount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(kUserId, kAsset, "0.0")),
              checkStatelessInvalid);
}

/**
 * @given pair of users with all required permissions
 * @when execute two txes with AddAssetQuantity command with amount more than a
 * uint256 max half
 * @then first transaction is committed and there is an empty proposal for the
 * second
 */
TEST_F(AddAssetQuantity, Uint256DestOverflow) {
  std::string uint256_halfmax =
      "723700557733226221397318656304299424082937404160253525246609900049457060"
      "2495.0";  // 2**252 - 1
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      // Add first half of the maximum
      .sendTx(
          complete(baseTx().addAssetQuantity(kUserId, kAsset, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Add second half of the maximum
      .sendTx(
          complete(baseTx().addAssetQuantity(kUserId, kAsset, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with AddAssetQuantity command with nonexistent account
 * @then there is an empty proposal
 */
TEST_F(AddAssetQuantity, NonexistentAccount) {
  std::string nonexistent = "inexist@test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(nonexistent, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with AddAssetQuantity command with nonexistent asset
 * @then there is an empty proposal
 */
TEST_F(AddAssetQuantity, NonexistentAsset) {
  std::string nonexistent = "inexist#test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(
          complete(baseTx().addAssetQuantity(kUserId, nonexistent, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permission
 * @when execute tx with AddAssetQuantity command to some other user
 * @then there is no tx in proposal
 */
TEST_F(AddAssetQuantity, OtherUser) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(
          IntegrationTestFramework::kAdminId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of user in different domains with all required permission
 * @when first one execute a tx with AddAssetQuantity command to the second one
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(AddAssetQuantity, OtherDomain) {
  const auto kNewRole = "newrl";
  const auto kNewDomain = "newdom";
  const auto kNewUser = "newusr";
  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      // Generate new domain, new user and an asset
      .sendTx(
          TestUnsignedTransactionBuilder()
              .creatorAccountId(
                  integration_framework::IntegrationTestFramework::kAdminId)
              .createdTime(iroha::time::now())
              .createRole(kNewRole,
                          std::vector<std::string>{
                              shared_model::permissions::can_get_my_txs})
              .createDomain(kNewDomain, kNewRole)
              .createAccount(
                  kNewUser,
                  kNewDomain,
                  crypto::DefaultCryptoAlgorithmType::generateKeypair()
                      .publicKey())
              .createAsset(IntegrationTestFramework::kAssetName, kNewDomain, 1)
              .quorum(1)
              .build()
              .signAndAddSignature(kAdminKeypair))
      .skipProposal()
      // Make sure everything is committed
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendTx(complete(baseTx().addAssetQuantity(kNewUser, kAsset, kAmount)),
              checkStatelessInvalid);
}
