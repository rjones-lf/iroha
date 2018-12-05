/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/query_responses/signatories_response.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

class AddSignatory : public AcceptanceFixture {
 public:
  auto makeFirstUser(const interface::RolePermissionSet &perms = {
                         interface::permissions::Role::kAddSignatory}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  auto makeSecondUser(const interface::RolePermissionSet &perms = {
                          interface::permissions::Role::kReceive}) {
    return complete(
        createUserWithPerms(kUser2, kUser2Keypair.publicKey(), kRole2, perms)
            .creatorAccountId(kAdminId),
        kAdminKeypair);
  }

  const std::string kRole2 = "roletwo";
  const std::string kUser2 = "usertwo";
  const std::string kUser2Id = kUser2 + "@" + kDomain;
  const std::string kAmount = "1.0";
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

#define CHECK_BLOCK(i) \
  [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

/**
 * C224 Add existing public key of other user
 * @given some user with CanAddSignatory permission and a second user
 * @when execute tx with AddSignatory where the first is a creator and the
 *       second's key is added as a signatory
 * @then there is the tx in proposal
 */
TEST_F(AddSignatory, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey())),
          CHECK_BLOCK(1))
      .sendQuery(
          complete(baseQry().creatorAccountId(kAdminId).getSignatories(kUserId),
                   kAdminKeypair),
          [this](auto &resp) {
            ASSERT_NO_THROW({
              auto &keys =
                  boost::apply_visitor(
                      framework::SpecifiedVisitor<
                          shared_model::interface::SignatoriesResponse>(),
                      resp.get())
                      .keys();
              ASSERT_EQ(keys.size(), 2);  // self + signatory
              ASSERT_TRUE(
                  std::find(keys.begin(), keys.end(), kUser2Keypair.publicKey())
                  != keys.end());
            });
          });
}

/**
 * C228 AddSignatory without such permissions
 * @given some user without CanAddSignatory permission and a second user
 * @when execute tx with AddSignatory where the first is a creator and the
 *       second's key is added as a signatory
 * @then there is the no tx in proposal
 */
TEST_F(AddSignatory, NoPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser({interface::permissions::Role::kReceive}),
                   CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTx(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey())))
      .checkVerifiedProposal(CHECK_BLOCK(0));
}

/**
 * C225 Add signatory to other user
 * C227 Add signatory to an account, which granted permission to add it, and add
 *      the same public key
 * @given some user with CanAddMySignatory permission and a second user with
          granted CanAddMySignatory
 * @when execute tx with AddSignatory where the second is a creator and the
 *       second's key is added as a signatory
 * @then there is the tx in proposal
 */
TEST_F(AddSignatory, GrantedPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeFirstUser({interface::permissions::Role::kAddMySignatory}),
          CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().grantPermission(
              kUser2Id, interface::permissions::Grantable::kAddMySignatory)),
          CHECK_BLOCK(1))
      .sendTxAwait(complete(baseTx().creatorAccountId(kUser2Id).addSignatory(
                                kUserId, kUser2Keypair.publicKey()),
                            kUser2Keypair),
                   CHECK_BLOCK(1));
}

/**
 * C226 Add signatory to account, which isn't granted such permission
 * @given some user with CanAddMySignatory permission and a second user without
          granted CanAddMySignatory
 * @when execute tx with AddSignatory where the second is a creator and the
 *       second's key is added as a signatory
 * @then there is no tx in proposal
 */
TEST_F(AddSignatory, NonGrantedPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeFirstUser({interface::permissions::Role::kAddMySignatory}),
          CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().creatorAccountId(kUser2Id).addSignatory(
                           kUserId, kUser2Keypair.publicKey()),
                       kUser2Keypair))
      .checkVerifiedProposal(CHECK_BLOCK(0));
}

/**
 * C222 Add signatory to non-existing account ID
 * @given some user with CanAddMySignatory permission
 * @when execute tx with AddSignatory with inexistent user
 * @then there is no tx in proposal
 */
TEST_F(AddSignatory, NonExistentUser) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().addSignatory("inexistent@" + kDomain,
                                             kUserKeypair.publicKey()),
                       kUser2Keypair))
      .checkVerifiedProposal(CHECK_BLOCK(0));
}

/**
 * C223 Add invalid public key
 * @given some user with CanAddMySignatory permission
 * @when execute tx with AddSignatory with inexistent public key
 * @then the tx is stateless invalid
 */
TEST_F(AddSignatory, InvalidKey) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().addSignatory(kUserId,
                                             shared_model::crypto::PublicKey(
                                                 std::string(1337, 'a'))),
                       kUser2Keypair),
              CHECK_STATELESS_INVALID);
}

/**
 * @given some user with CanAddMySignatory permission
 * @when execute tx with AddSignatory with a valid key which isn't associated
 *       with any user
 * @then there is no tx in proposal
 */
TEST_F(AddSignatory, NonExistedKey) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(
          baseTx().addSignatory(
              kUserId, shared_model::crypto::PublicKey(std::string(32, 'a'))),
          kUser2Keypair))
      .checkVerifiedProposal(CHECK_BLOCK(0));
}
