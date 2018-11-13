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

class RemoveSignatory : public AcceptanceFixture {
 public:
  auto makeFirstUser(const interface::RolePermissionSet &perms = {
                         interface::permissions::Role::kRemoveSignatory}) {
    auto new_perms = perms;
    new_perms.set(interface::permissions::Role::kAddSignatory);
    return AcceptanceFixture::makeUserWithPerms(new_perms);
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
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

#define CHECK_BLOCK(i) \
  [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

/**
 * C264 Remove signatory from own account
 * C267 Remove signatory more than once
 * @given some user with CanRemoveSignatory permission and its signatory
 * @when execute tx with RemoveSignatory where the first is a creator and the
 *       second's key is removed as a signatory
 * @then the first such tx is committed,
         the same transaction afterward shouldn't pass stateful validation
 */
TEST_F(RemoveSignatory, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey())),
          CHECK_BLOCK(1))
      .sendTxAwait(complete(baseTx().removeSignatory(
                       kUserId, kUser2Keypair.publicKey())),
                   CHECK_BLOCK(1))
      .sendTx(complete(
          baseTx().removeSignatory(kUserId, kUser2Keypair.publicKey())))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}

/**
 * C263 RemoveSignatory without such permissions
 * @given some user without CanRemoveSignatory permission and its signatory
 * @when execute tx with RemoveSignatory where the first is a creator and the
 *       second's key is removed as a signatory
 * @then there is the no tx in proposal
 */
TEST_F(RemoveSignatory, NoPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser({}), CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey()),
                   kUserKeypair),
          CHECK_BLOCK(1))
      .sendTx(complete(
          baseTx().removeSignatory(kUserId, kUser2Keypair.publicKey())))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}

/**
 * C265 Remove signatory from granted account
 * @given some user with CanRemoveMySignatory permission and its signatory with
 *        granted CanRemoveMySignatory
 * @when execute tx with RemoveSignatory where the second is a creator and the
 *       second's key is removed as a signatory
 * @then there is the tx in proposal
 */
TEST_F(RemoveSignatory, GrantedPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeFirstUser({interface::permissions::Role::kRemoveMySignatory}),
          CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey()),
                   kUserKeypair),
          CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().grantPermission(
              kUser2Id, interface::permissions::Grantable::kRemoveMySignatory)),
          CHECK_BLOCK(1))
      .sendTxAwait(complete(baseTx().creatorAccountId(kUser2Id).removeSignatory(
                                kUserId, kUser2Keypair.publicKey()),
                            kUser2Keypair),
                   CHECK_BLOCK(1));
}

/**
 * @given some user with CanRemoveMySignatory permission and its signatory
 *        without granted CanRemoveMySignatory
 * @when execute tx with RemoveSignatory where the second is a creator and the
 *       second's key is removed as a signatory
 * @then there is no tx in proposal
 */
TEST_F(RemoveSignatory, NonGrantedPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeFirstUser({interface::permissions::Role::kRemoveMySignatory}),
          CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey()),
                   kUserKeypair),
          CHECK_BLOCK(1))
      .sendTx(complete(baseTx().creatorAccountId(kUser2Id).removeSignatory(
                           kUserId, kUser2Keypair.publicKey()),
                       kUser2Keypair))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}

/**
 * @given some user with CanRemoveSignatory permission
 * @when execute tx with RemoveSignatory with inexistent user
 * @then there is no tx in proposal
 */
TEST_F(RemoveSignatory, NonExistentUser) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().removeSignatory("inexistent@" + kDomain,
                                                kUserKeypair.publicKey()),
                       kUser2Keypair))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}

/**
 * C266 Remove signatory with an invalid public key
 * @given some user with CanRemoveSignatory permission
 * @when execute tx with RemoveSignatory with invalid public key
 * @then the tx is stateless invalid
 */
TEST_F(RemoveSignatory, InvalidKey) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().removeSignatory(
                  kUserId,
                  shared_model::crypto::PublicKey(std::string(1337, 'a')))),
              CHECK_STATELESS_INVALID);
}

/**
 * @given some user with CanRemoveSignatory permission
 * @when execute tx with RemoveSignatory with a key which isn't associated with
 *       any user
 * @then there is no tx in proposal
 */
TEST_F(RemoveSignatory, NonExistedKey) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTx(complete(baseTx().removeSignatory(
          kUserId, shared_model::crypto::PublicKey(std::string(32, 'a')))))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}

/**
 * C268 Remove signatory so that account may have less signatories than the
 *      quorum
 * @given some user with CanRemoveSignatory permission, which account quorum is
 * 2 AND its signatory
 * @when execute tx with RemoveSignatory where the first is a creator and the
 *       second's key is removed as a signatory
 * @then there is no tx in proposal
 * TODO: SetQuroum permission issue, enable after IR-920
 */
TEST_F(RemoveSignatory, DISABLED_SignatoriesLesserThanQuorum) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), CHECK_BLOCK(1))
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendTxAwait(
          complete(baseTx().addSignatory(kUserId, kUser2Keypair.publicKey())),
          CHECK_BLOCK(1))
      .sendTxAwait(
          complete(
              baseTx().creatorAccountId(kAdminId).setAccountQuorum(kUserId, 2),
              kAdminKeypair),
          CHECK_BLOCK(1))
      .sendTx(complete(
          baseTx().removeSignatory(kUserId, kUser2Keypair.publicKey())))
      .checkVerifiedProposal(CHECK_BLOCK(0))
      .checkBlock(CHECK_BLOCK(0));
}
