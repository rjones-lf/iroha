/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

class CreateAccount : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kCreateAccount}) {
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
      .sendTxAwait(
          complete(baseTx().createAccount(
              kNewUser, kDomain, kNewUserKeypair.publicKey())),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
}

/**
 * @given some user without can_create_account permission
 * @when execute tx with CreateAccount command
 * @then verified proposal is empty
 */
TEST_F(CreateAccount, NoPermissions) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kGetMyTxs}))
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          kNewUser, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto block) { ASSERT_EQ(block->transactions().size(), 0); });
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with nonexistent domain
 * @then verified proposal is empty
 */
TEST_F(CreateAccount, NoDomain) {
  const std::string nonexistent_domain = "asdf";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          kNewUser, nonexistent_domain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto block) { ASSERT_EQ(block->transactions().size(), 0); });
}

/**
 * @given some user with can_create_account permission
 * @when execute tx with CreateAccount command with already existing username
 * @then verified proposal is empty
 */
TEST_F(CreateAccount, ExistingName) {
  std::string existing_name = kUser;
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createAccount(
          existing_name, kDomain, kNewUserKeypair.publicKey())))
      .skipProposal()
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](const auto block) { ASSERT_EQ(block->transactions().size(), 0); });
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
      .sendTxAwait(
          complete(baseTx().createAccount(
              std::string(32, 'a'), kDomain, kNewUserKeypair.publicKey())),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
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
              CHECK_STATELESS_INVALID);
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
              CHECK_STATELESS_INVALID);
}
