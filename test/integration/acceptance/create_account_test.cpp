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
#include "validators/permissions.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class CreateAccount : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             iroha::model::can_create_account}) {
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
  const std::string kNewUser = "userone"s;
  const std::string kDomain = IntegrationTestFramework::kDefaultDomain;
  const std::string kUserId = kUser + "@" + kDomain;
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kNewUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command
 * @then there is the tx in proposal
 */
TEST_F(CreateAccount, Basic) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          kNewUser, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user without can_create_account permission
 * @when execute tx with CreateAccount command
 * @then there is no tx in proposal
 */
TEST_F(CreateAccount, NoPermissions) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          kNewUser, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with inexistent domain
 * @then there is no tx in proposal
 */
TEST_F(CreateAccount, NoDomain) {
  const std::string inexistent_domain = "asdf"s;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          kNewUser, inexistent_domain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with already existing username
 * @then there is no tx in proposal
 */
TEST_F(CreateAccount, ExistentName) {
  const std::string &existent_name = kUser;
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          existent_name, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with maximum available length
 * @then there is the tx in proposal
 */
TEST_F(CreateAccount, MaxLenName) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          std::string(32, 'a'), kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with too long length
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateAccount, TooLongName) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          std::string(33, 'a'), kDomain, kNewUserKeypair.publicKey())));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with empty user name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateAccount, EmptyName) {
  const std::string &empty_name = "";
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().createAccount(
          empty_name, kDomain, kNewUserKeypair.publicKey())));
  ASSERT_ANY_THROW(itf.skipProposal());
}
