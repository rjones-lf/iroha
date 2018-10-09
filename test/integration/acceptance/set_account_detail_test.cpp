/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "builders/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"

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
 * C274
 * @given a user without can_set_detail permission
 * @when execute tx with SetAccountDetail command aimed to the user
 * @then there is the tx in block
 */
TEST_F(SetAccountDetail, Self) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTxAwait(complete(baseTx(kUserId)), [](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1);
      });
}

/**
 * C273
 * @given a user with required permission
 * @when execute tx with SetAccountDetail command with inexistent user
 * @then there is no tx in block
 */
TEST_F(SetAccountDetail, NonExistentUser) {
  const std::string kInexistent = "inexistent@" + kDomain;
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kSetDetail}))
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          complete(baseTx(kInexistent, kKey, kValue)),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); });
}

/**
 * C280
 * @given a pair of users and first one without permissions
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is an empty verified proposal
 */
TEST_F(SetAccountDetail, WithoutNoPerm) {
  auto second_user_tx = makeSecondUser();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTxAwait(second_user_tx,
                   [](auto &block) {
                     ASSERT_EQ(block->transactions().size(), 1)
                         << "Cannot create second user account";
                   })
      .sendTx(complete(baseTx(kUser2Id)))
      .skipProposal()
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto block) { ASSERT_EQ(block->transactions().size(), 0); });
}

/**
 * @given a pair of users and first one with can_set_detail perm
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is the tx in block
 */
TEST_F(SetAccountDetail, WithPerm) {
  auto second_user_tx = makeSecondUser();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kSetDetail}))
      .skipProposal()
      .skipBlock()
      .sendTxAwait(second_user_tx,
                   [](auto &block) {
                     ASSERT_EQ(block->transactions().size(), 1)
                         << "Cannot create second user account";
                   })
      .sendTxAwait(complete(baseTx(kUser2Id)), [](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1);
      });
}

/**
 * C275
 * @given a pair of users
 *        @and second has been granted can_set_my_detail from the first
 * @when the first one tries to use SetAccountDetail on the second
 * @then there is an empty verified proposal
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
      .sendTxAwait(second_user_tx,
                   [](auto &block) {
                     ASSERT_EQ(block->transactions().size(), 1)
                         << "Cannot create second user account";
                   })
      .sendTxAwait(complete(AcceptanceFixture::baseTx().grantPermission(
                       kUser2Id,
                       interface::permissions::Grantable::kSetMyAccountDetail)),
                   [](auto &block) {
                     ASSERT_EQ(block->transactions().size(), 1)
                         << "Cannot grant permission";
                   })
      .sendTxAwait(set_detail_cmd, [](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1);
      });
}

/**
 * C276
 * @given a user with required permission
 * @when execute tx with SetAccountDetail command with max key
 * @then there is the tx in block
 */
TEST_F(SetAccountDetail, BigPossibleKey) {
  const std::string kBigKey = std::string(64, 'a');
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTxAwait(complete(baseTx(kUserId, kBigKey, kValue)), [](auto &block) {
        ASSERT_EQ(block->transactions().size(), 1);
      });
}

/**
 * C277
 * @given a user with required permission
 * @when execute tx with SetAccountDetail command with empty key
 * @then there is no tx in block
 */
TEST_F(SetAccountDetail, EmptyKey) {
  const std::string kEmptyKey = "";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx(kUserId, kEmptyKey, kValue)),
              checkStatelessInvalid);
}

/**
 * C278
 * @given a user with required permission
 * @when execute tx with SetAccountDetail command with empty value
 * @then there is no tx in block
 */
TEST_F(SetAccountDetail, EmptyValue) {
  const std::string kEmptyValue = "";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          complete(baseTx(kUserId, kKey, kEmptyValue)),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
}

/**
 * C279
 * @given a user with required permission
 * @when execute tx with SetAccountDetail command with huge both key and value
 * @then there is no tx in block
 */
TEST_F(SetAccountDetail, HugeKeyValue) {
  const std::string kHugeKey = std::string(10000, 'a');
  const std::string kHugeValue = std::string(10000, 'b');
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(complete(baseTx(kUserId, kHugeKey, kHugeValue)),
              checkStatelessInvalid);
}
