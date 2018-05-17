/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

class SubtractAssetQuantity : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_subtract_asset_qty,
                             shared_model::permissions::can_add_asset_qty}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  /**
   * @return built tx that adds kAmount assets to the users
   */
  auto replenish() {
    return complete(baseTx().addAssetQuantity(kUserId, kAsset, kAmount));
  }

  const std::string kAmount = "1.0";
};

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with max available amount
 * @then there is the tx in proposal
 */
TEST_F(SubtractAssetQuantity, Everything) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          complete(baseTx().subtractAssetQuantity(kUserId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with amount more than
 * user has
 * @then there is no tx in proposal
 */
TEST_F(SubtractAssetQuantity, Overdraft) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().subtractAssetQuantity(kUserId, kAsset, "2.0")))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user without can_subtract_asset_qty permission
 * @when execute tx with SubtractAssetQuantity command
 * @then there is no tx in proposal
 */
TEST_F(SubtractAssetQuantity, NoPermissions) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_add_asset_qty}))
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          complete(baseTx().subtractAssetQuantity(kUserId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with SubtractAssetQuantity command with negative amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(SubtractAssetQuantity, NegativeAmount) {
  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().subtractAssetQuantity(kUserId, kAsset, "-1.0")),
              checkStatelessInvalid);
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with SubtractAssetQuantity command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(SubtractAssetQuantity, ZeroAmount) {
  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().subtractAssetQuantity(kUserId, kAsset, "0.0")),
              checkStatelessInvalid);
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with nonexitent account
 * @then there is an empty proposal
 */
TEST_F(SubtractAssetQuantity, NonexistentAccount) {
  std::string nonexistent = "inexist@test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(
          baseTx().subtractAssetQuantity(nonexistent, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with nonexistent asset
 * @then there is an empty proposal
 */
TEST_F(SubtractAssetQuantity, NonexistentAsset) {
  std::string nonexistent = "inexist#test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(
          baseTx().subtractAssetQuantity(kUserId, nonexistent, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command to some other user
 * @then there is no tx in proposal
 */
TEST_F(SubtractAssetQuantity, OtherUser) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().subtractAssetQuantity(
          IntegrationTestFramework::kAdminId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}
