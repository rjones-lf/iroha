/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;

class SetAccountDetail : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kAddPeer}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  auto baseTx(const interface::types::AccountIdType &account_id,
              const interface::types::AccountDetailKeyType &key,
              const interface::types::AccountDetailValueType &value) {
    return AcceptanceFixture::baseTx().setAccountDetail(account_id, key, value);
  }

  auto baseTx(const interface::types::AccountIdType &account_id) {
    return baseTx(account_id, kKey, kValue);
  }

  auto makeSecondUser(const interface::RolePermissionSet &perms = {
                          interface::permissions::Role::kAddPeer}) {
    static const std::string kRole2 = "roletwo";
    return AcceptanceFixture::createUserWithPerms(
               kUser2, kUser2Keypair.publicKey(), kRole2, perms)
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  const interface::types::AccountDetailKeyType kKey = "key";
  const interface::types::AccountDetailValueType kValue = "value";
  const std::string kUser2 = "user2";
  const std::string kUser2Id =
      kUser2 + "@" + IntegrationTestFramework::kDefaultDomain;
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given a user without can_set_detail permission
 * @when execute tx with SetAccountDetail command aimed to the user
 * @then there is the tx in proposal
 */
TEST_F(SetAccountDetail, Self) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx(kUserId)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given a pair of users and first one without permissions
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is the tx in proposal
 */
TEST_F(SetAccountDetail, WithoutNoPerm) {
  auto second_user_tx = makeSecondUser();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(second_user_tx)
      .skipProposal()
      .checkBlock([](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1)
            << "Cannot create second user account";
      })
      .sendTx(complete(baseTx(kUser2Id)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given a pair of users and first one with can_set_detail perm
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is the tx in proposal
 */
TEST_F(SetAccountDetail, WithPerm) {
  auto second_user_tx = makeSecondUser();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kSetDetail}))
      .skipProposal()
      .skipBlock()
      .sendTx(second_user_tx)
      .skipProposal()
      .checkBlock([](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1)
            << "Cannot create second user account";
      })
      .sendTx(complete(baseTx(kUser2Id)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * @given a pair of users
 *        AND second has been granted can_set_my_detail from the first
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is no tx in proposal
 */
TEST_F(SetAccountDetail, WithGrantablePerm) {
  auto second_user_tx = makeSecondUser();
  auto set_detail_cmd = baseTx(kUserId)
                            .creatorAccountId(kUser2Id)
                            .build()
                            .signAndAddSignature(kUser2Keypair)
                            .finish();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(
          {interface::permissions::Role::kSetMyAccountDetail}))
      .skipProposal()
      .skipBlock()
      .sendTx(second_user_tx)
      .skipProposal()
      .checkBlock([](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1)
            << "Cannot create second user account";
      })
      .sendTx(complete(AcceptanceFixture::baseTx().grantPermission(
          kUser2Id, interface::permissions::Grantable::kSetMyAccountDetail)))
      .skipProposal()
      .checkBlock([](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1) << "Cannot grant permission";
      })
      .sendTx(set_detail_cmd)
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}
