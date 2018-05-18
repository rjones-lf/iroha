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

class CreateAccount : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_create_account}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  const std::string kNewUser = "userone";
  const crypto::Keypair kNewUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command
 * @then there is the tx in proposal
 */
TEST_F(CreateAccount, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
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
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          kNewUser, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with nonexistent domain
 * @then there is no tx in proposal
 */
TEST_F(CreateAccount, NoDomain) {
  const std::string nonexistent_domain = "asdf";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          kNewUser, nonexistent_domain, kNewUserKeypair.publicKey())))
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
TEST_F(CreateAccount, ExistingName) {
  std::string existing_name = kUser;
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          existing_name, kDomain, kNewUserKeypair.publicKey())))
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
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
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
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
                  std::string(33, 'a'), kDomain, kNewUserKeypair.publicKey())),
              checkStatelessInvalid);
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with empty user name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateAccount, EmptyName) {
  std::string empty_name = "";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
                  empty_name, kDomain, kNewUserKeypair.publicKey())),
              checkStatelessInvalid);
}
