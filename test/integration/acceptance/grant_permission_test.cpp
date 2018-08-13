/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/grantable_permissions_fixture.hpp"

using namespace integration_framework;

using namespace shared_model;
using namespace shared_model::interface;
using namespace shared_model::interface::permissions;

/**
 * C256 Grant permission to a non-existing account
 * @given an account with rights to grant rights to other accounts
 * @when the account grants rights to non-existing account
 * @then this transaction is stateful invalid
 * AND it is not written in the block
 */
TEST_F(GrantablePermissionsFixture, GrantToInexistingAccount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeAccountWithPerms(
          kAccount1, kAccount1Keypair, kCanGrantAll, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * C257 Grant add signatory permission
 * @given an account with rights to grant rights to other accounts
 * AND the account grants add signatory rights to an existing account (permitee)
 * @when the permitee adds signatory to the account
 * @then a block with transaction to add signatory to the account is written
 * AND there is a signatory added by the permitee
 */
TEST_F(GrantablePermissionsFixture, GrantAddSignatoryPermission) {
  auto expected_number_of_signatories = 2;
  auto is_contained = true;
  auto check_if_signatory_is_contained = checkSignatorySet(
      kAccount2Keypair, expected_number_of_signatories, is_contained);

  IntegrationTestFramework itf(1);
  auto &x = createTwoAccounts(
      itf, {Role::kAddMySignatory, Role::kGetMySignatories}, {Role::kReceive});
  x.sendTx(accountGrantToAccount(kAccount1,
                                 kAccount1Keypair,
                                 kAccount2,
                                 permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Add signatory
      .sendTx(
          permiteeModifySignatory(&TestUnsignedTransactionBuilder::addSignatory,
                                  kAccount2,
                                  kAccount2Keypair,
                                  kAccount1))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(querySignatories(kAccount1, kAccount1Keypair),
                 check_if_signatory_is_contained)
      .done();
}

/**
 * C258 Grant remove signatory permission
 * @given an account with rights to grant rights to other accounts
 * AND the account grants add and remove signatory rights to an existing account
 * AND the permittee has added his/her signatory to the account
 * AND the account revoked add signatory from the permittee
 * @when the permittee removes signatory from the account
 * @then a block with transaction to remove signatory from the account is
 * written AND there is no signatory added by the permitee
 */
TEST_F(GrantablePermissionsFixture, GrantRemoveSignatoryPermission) {
  auto expected_number_of_signatories = 1;
  auto is_contained = false;
  auto check_if_signatory_is_not_contained = checkSignatorySet(
      kAccount2Keypair, expected_number_of_signatories, is_contained);

  IntegrationTestFramework itf(1);
  createTwoAccounts(itf,
                    {Role::kAddMySignatory,
                     Role::kRemoveMySignatory,
                     Role::kGetMySignatories},
                    {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .skipBlock()
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kRemoveMySignatory))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(
          permiteeModifySignatory(&TestUnsignedTransactionBuilder::addSignatory,
                                  kAccount2,
                                  kAccount2Keypair,
                                  kAccount1))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(permiteeModifySignatory(
          &TestUnsignedTransactionBuilder::removeSignatory,
          kAccount2,
          kAccount2Keypair,
          kAccount1))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(querySignatories(kAccount1, kAccount1Keypair),
                 check_if_signatory_is_not_contained)
      .done();
}

/**
 * C259 Grant set quorum permission
 * @given an account with rights to grant rights to other accounts
 * AND the account grants add signatory rights
 * AND set quorum rights to an existing account
 * AND the permittee has added his/her signatory to the account
 * @when the permittee changes the number of quorum in the account
 * @then a block with transaction to change quorum in the account is written
 * AND the quorum number of account equals to the number, set by permitee
 */
TEST_F(GrantablePermissionsFixture, GrantSetQuorumPermission) {
  auto quorum_quantity = 2;
  auto check_quorum_quantity = checkQuorum(quorum_quantity);

  IntegrationTestFramework itf(1);
  createTwoAccounts(
      itf,
      {Role::kSetMyQuorum, Role::kAddMySignatory, Role::kGetMyAccount},
      {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kSetMyQuorum))
      .skipProposal()
      .skipBlock()
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .skipBlock()
      .sendTx(
          permiteeModifySignatory(&TestUnsignedTransactionBuilder::addSignatory,
                                  kAccount2,
                                  kAccount2Keypair,
                                  kAccount1))
      .skipProposal()
      .skipBlock()
      .sendTx(permiteeSetQuorum(kAccount2, kAccount2Keypair, kAccount1, 2))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(queryAccount(kAccount1, kAccount1Keypair),
                 check_quorum_quantity)
      .done();
}

/**
 * C260 Grant set account detail permission
 * @given an account with rights to grant rights to other accounts
 * AND the account grants set account detail permission to a permitee
 * @when the permittee sets account detail to the account
 * @then a block with transaction to set account detail is written
 * AND the permitee is able to read the data
 * AND the account is able to read the data
 */
TEST_F(GrantablePermissionsFixture, GrantSetAccountDetailPermission) {
  auto check_account_detail =
      checkAccountDetail(kAccountDetailKey, kAccountDetailValue);

  IntegrationTestFramework itf(1);
  createTwoAccounts(
      itf, {Role::kSetMyAccountDetail, Role::kGetMyAccDetail}, {Role::kReceive})
      .sendTx(
          accountGrantToAccount(kAccount1,
                                kAccount1Keypair,
                                kAccount2,
                                permissions::Grantable::kSetMyAccountDetail))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(permiteeSetAccountDetail(kAccount2,
                                       kAccount2Keypair,
                                       kAccount1,
                                       kAccountDetailKey,
                                       kAccountDetailValue))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(queryAccountDetail(kAccount1, kAccount1Keypair),
                 check_account_detail)
      .done();
}

/**
 * C261 Grant transfer permission
 * @given an account with rights to grant transfer of his/her assets
 * AND the account can receive assets
 * AND the account has some amount of assets
 * AND the account has permitted to some other account in the system
 * to transfer his/her assets
 * @when the permitee transfers assets of the account
 * @then a block with transaction to grant right is written
 * AND the transfer is made
 */
TEST_F(GrantablePermissionsFixture, GrantTransferPermission) {
  auto amount_of_asset = "1000.0";

  IntegrationTestFramework itf(1);
  createTwoAccounts(itf,
                    {Role::kTransferMyAssets, Role::kReceive},
                    {Role::kTransfer, Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kTransferMyAssets))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(addAssetAndTransfer(IntegrationTestFramework::kAdminName,
                                  kAdminKeypair,
                                  amount_of_asset,
                                  kAccount1))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(transferAssetFromSource(
          kAccount2, kAccount2Keypair, kAccount1, amount_of_asset, kAccount2))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * C262 GrantPermission without such permissions
 * @given an account !without! rights to grant rights to other accounts
 * @when the account grants rights to an existing account
 * @then this transaction is statefully invalid
 * AND it is not written in the block
 */
TEST_F(GrantablePermissionsFixture, GrantWithoutGrantPermissions) {
  IntegrationTestFramework itf(1);
  createTwoAccounts(itf, {Role::kReceive}, {Role::kReceive});
  for (auto &perm : kAllGrantable) {
    itf.sendTx(
           accountGrantToAccount(kAccount1, kAccount1Keypair, kAccount2, perm))
        .skipProposal()
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); });
  }
  itf.done();
}

/**
 * C263 GrantPermission more than once
 * @given an account with rights to grant rights to other accounts
 * AND an account that have already granted a permission to a permittee
 * @when the account grants the same permission to the same permittee
 * @then this transaction is statefully invalid
 * AND it is not written in the block
 */

TEST_F(GrantablePermissionsFixture, GrantMoreThanOnce) {
  IntegrationTestFramework itf(1);
  createTwoAccounts(itf, {kCanGrantAll}, {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .skipBlock()
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kAddMySignatory))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}
