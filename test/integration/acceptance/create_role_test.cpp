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
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class CreateRole : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_get_my_txs,
                             shared_model::permissions::can_create_role}) {
    return framework::createUserWithPerms(
               kUser, kUserKeypair.publicKey(), kNewRole, perms)
        .build()
        .signAndAddSignature(kAdminKeypair);
  }

  /**
   * Create valid base pre-built transaction with CreateRole command
   * @param perms is a permission list
   * @param role_name is a name of the role
   * @return pre-built tx
   */
  auto baseTx(const std::vector<std::string> &perms,
              const std::string &role_name) {
    return TestUnsignedTransactionBuilder()
        .createRole(role_name, perms)
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now());
  }

  auto baseTx(const std::vector<std::string> &perms = {
                  shared_model::permissions::can_get_my_txs}) {
    return baseTx(perms, kRole);
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

  const std::string kRole = "role"s;
  const std::string kUser = "user"s;
  const std::string kNewRole = "rl"s;
  const std::string kUserId = kUser + "@test";
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command
 * @then there is the tx in proposal
 */
TEST_F(CreateRole, Basic) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx()))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user without can_create_role permission
 * @when execute tx with CreateRole command
 * @then there is an empty verified proposal
 */
TEST_F(CreateRole, HaveNoPerms) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx()))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with empty role
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateRole, EmptyRole) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(
          completeTx(baseTx({shared_model::permissions::can_get_my_txs}, "")));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with empty permission
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateRole, EmptyPerms) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx({})));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with too long role name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateRole, LongRoleName) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx({shared_model::permissions::can_get_my_txs},
                                std::string(33, 'a'))));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with maximal role name size
 * @then the tx is comitted
 */
TEST_F(CreateRole, MaxLenRoleName) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx({shared_model::permissions::can_get_my_txs},
                                std::string(32, 'a'))))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with inexistent permission name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateRole, DISABLED_InexistentPerm) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx({"this_permission_doesnt_exist"})));
  ASSERT_ANY_THROW(itf.skipProposal());
}

/**
 * @given some user with can_create_role permission
 * @when execute tx with CreateRole command with existent role name
 * @then there is an empty verified proposal
 */
TEST_F(CreateRole, DISABLED_ExistingRole) {
  IntegrationTestFramework itf;
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx()));
  ASSERT_ANY_THROW(itf.skipProposal());
}
