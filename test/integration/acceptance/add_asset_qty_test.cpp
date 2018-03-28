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

class AddAssetQuantity : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
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
 * @given some user with can_add_asset_qty permission
 * @when execute tx with AddAssetQuantity command
 * @then there is the tx in proposal
 */
TEST_F(AddAssetQuantity, Basic) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().addAssetQuantity(kUserId, kAsset, kAmount)))
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().addAssetQuantity(kUserId, kAsset, kAmount)))
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
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().addAssetQuantity(kUserId, kAsset, "-1.0")));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with AddAssetQuantity command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(AddAssetQuantity, ZeroAmount) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().addAssetQuantity(kUserId, kAsset, "0.0")));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given pair of users with all required permissions
 * @when execute two txes with AddAssetQuantity command with amount more than a
 * uint256 max half
 * @then first transaction is committed and there is an empty proposal for the
 * second
 */
TEST_F(AddAssetQuantity, Uint256DestOverflow) {
  const std::string &uint256_halfmax =
      "723700557733226221397318656304299424082937404160253525246609900049457060"
      "2495.0";  // 2**252 - 1
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      // Add first half of the maximum
      .sendTx(completeTx(
          baseTx().addAssetQuantity(kUserId, kAsset, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Add second half of the maximum
      .sendTx(completeTx(
          baseTx().addAssetQuantity(kUserId, kAsset, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with AddAssetQuantity command with inexistent account
 * @then there is an empty proposal
 */
TEST_F(AddAssetQuantity, InexistentAccount) {
  const std::string &inexistent = "inexist@test"s;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().addAssetQuantity(inexistent, kAsset, kAmount)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with AddAssetQuantity command with inexistent asset
 * @then there is an empty proposal
 */
TEST_F(AddAssetQuantity, InexistentAsset) {
  const std::string &inexistent = "inexist#test"s;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx().addAssetQuantity(kUserId, inexistent, kAmount)))
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().addAssetQuantity(
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
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      // Generate new domain, new user and an asset
      .sendTx(
          shared_model::proto::TransactionBuilder()
              .txCounter(1)
              .creatorAccountId(
                  integration_framework::IntegrationTestFramework::kAdminId)
              .createdTime(iroha::time::now())
              .createRole(
                  kNewRole,
                  std::vector<std::string>{iroha::model::can_get_my_txs})
              .createDomain(kNewDomain, kNewRole)
              .createAccount(
                  kNewUser,
                  kNewDomain,
                  crypto::DefaultCryptoAlgorithmType::generateKeypair()
                      .publicKey())
              .createAsset(IntegrationTestFramework::kAssetName, kNewDomain, 1)
              .build()
              .signAndAddSignature(kAdminKeypair))
      .skipProposal()
      // Make sure everything is committed
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendTx(completeTx(baseTx().addAssetQuantity(kNewUser, kAsset, kAmount)));
  ASSERT_ANY_THROW(itf.skipProposal());
}
