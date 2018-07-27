/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;

class AddAssetQuantity : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kAddAssetQty}) {
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
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, kAmount)))
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
      .sendTx(makeUserWithPerms({interface::permissions::Role::kGetMyTxs}))
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, kAmount)))
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
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, "-1.0")),
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
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, "0.0")),
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
      "578960446186580977117854925043439539266349923328202820197287920039565648"
      "19966.0";  // 2**255 - 2
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      // Add first half of the maximum
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Add second half of the maximum
      .sendTx(complete(baseTx().addAssetQuantity(kAsset, uint256_halfmax)))
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
      .sendTx(complete(baseTx().addAssetQuantity(nonexistent, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}
