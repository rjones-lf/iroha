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

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/base_tx.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "model/permissions.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class SubtractAssetQuantity : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             iroha::model::can_subtract_asset_qty,
                             iroha::model::can_add_asset_qty}) {
    return framework::createUserWithPerms(
               kUser, kUserKeypair.publicKey(), "role"s, perms)
        .build()
        .signAndAddSignature(kAdminKeypair);
  }

  /**
   * Create valid base pre-built transaction
   * @return pre-built tx
   */
  auto baseTx() {
    return TestUnsignedTransactionBuilder()
        .txCounter(1)
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now());
  }

  /**
   * Completes pre-built transaction
   * @param builder is a pre-built tx
   * @return built tx
   */
  template <typename TestTransactionBuilder>
  auto completeTx(TestTransactionBuilder builder) {
    return builder.build().signAndAddSignature(kUserKeypair);
  }

  /**
   * @return built tx that adds kAmount assets to the users
   */
  auto replenish() {
    return completeTx(baseTx().addAssetQuantity(kUserId, kAsset, kAmount));
  }

  const std::string kUser = "user"s;
  const std::string kAsset = IntegrationTestFramework::kAssetName + "#test";
  const std::string kUserId = kUser + "@test";
  const std::string kAmount = "1.0";
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with max available amount
 * @then there is the tx in proposal
 */
TEST_F(SubtractAssetQuantity, Everything) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().subtractAssetQuantity(kUserId, kAsset, kAmount)))
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().subtractAssetQuantity(kUserId, kAsset, "2.0")))
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_add_asset_qty}))
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().subtractAssetQuantity(kUserId, kAsset, kAmount)))
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
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().subtractAssetQuantity(kUserId, kAsset, "-1.0")));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with SubtractAssetQuantity command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(SubtractAssetQuantity, ZeroAmount) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().subtractAssetQuantity(kUserId, kAsset, "0.0")));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with inexistent account
 * @then there is an empty proposal
 */
TEST_F(SubtractAssetQuantity, InexistentAccount) {
  const std::string &inexistent = "inexist@test"s;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(
          baseTx().subtractAssetQuantity(inexistent, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with SubtractAssetQuantity command with inexistent asset
 * @then there is an empty proposal
 */
TEST_F(SubtractAssetQuantity, InexistentAsset) {
  const std::string &inexistent = "inexist#test"s;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(
          baseTx().subtractAssetQuantity(kUserId, inexistent, kAmount)))
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(replenish())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().subtractAssetQuantity(
          IntegrationTestFramework::kAdminId, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}
