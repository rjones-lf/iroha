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

class CreateDomain : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_create_domain}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  const std::string kNewDomain = "newdomain";
};

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command
 * @then there is the tx in proposal
 */
TEST_F(CreateDomain, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(kNewDomain, kRole)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user without can_create_domain permission
 * @when execute tx with CreateDomain command
 * @then there is no tx in proposal
 */
TEST_F(CreateDomain, NoPermissions) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_get_my_txs}))
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(kNewDomain, kRole)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with nonexistent role
 * @then there is no tx in proposal
 */
TEST_F(CreateDomain, NoRole) {
  const std::string nonexistent_role = "asdf";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(kNewDomain, nonexistent_role)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with already existing domain
 * @then there is no tx in proposal
 */
TEST_F(CreateDomain, ExistingName) {
  std::string existing_domain = IntegrationTestFramework::kDefaultDomain;
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(existing_domain, kRole)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with maximum available length
 * @then there is the tx in proposal
 */
TEST_F(CreateDomain, MaxLenName) {
  std::string maxLongDomain =
      // 255 characters string
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(maxLongDomain, kRole)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with too long length
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateDomain, TooLongName) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(std::string(257, 'a'), kRole)),
              checkStatelessInvalid);
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with empty domain name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateDomain, EmptyName) {
  std::string empty_name = "";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(empty_name, kRole)),
              checkStatelessInvalid);
}

/**
 * @given some user with can_create_domain permission
 * @when execute tx with CreateDomain command with empty role name
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(CreateDomain, DISABLED_EmptyRoleName) {
  std::string empty_name = "";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx().createDomain(kNewDomain, empty_name)),
              checkStatelessInvalid);
}
