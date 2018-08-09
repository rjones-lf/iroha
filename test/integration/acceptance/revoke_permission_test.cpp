/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/grant_fixture.hpp"

#include "builders/protobuf/builder_templates/query_template.hpp"

/**
 * C269 Revoke permission from a non-existing account
 * @given ITF instance and only one account with can_grant permission
 * @when the account tries to revoke grantable permission from non-existing
 * account
 * @then transaction would not be committed
 */
TEST_F(GrantPermissionFixture, RevokeFromNonExistingAccount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeAccountWithPerms(
          kAccount1, kAccount1Keypair, {Role::kSetMyQuorum}, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(accountRevokeFromAccount(kAccount1,
                                       kAccount1Keypair,
                                       kAccount2,
                                       permissions::Grantable::kSetMyQuorum))
      .checkProposal(
          // transaction is stateless valid
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          // transaction is not stateful valid (kAccount2 does not exist)
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * C271 Revoke permission more than once
 * @given ITF instance, two accounts, the first account has granted a permission
 * to the second
 * @when the first account revokes the permission twice
 * @then the second revoke does not pass stateful validation
 */
TEST_F(GrantPermissionFixture, RevokeTwice) {
  IntegrationTestFramework itf(1);
  createTwoAccounts(itf, {Role::kSetMyQuorum}, {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kSetMyQuorum))
      .checkBlock(
          // permission was successfully granted
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(accountRevokeFromAccount(kAccount1,
                                       kAccount1Keypair,
                                       kAccount2,
                                       permissions::Grantable::kSetMyQuorum))
      .checkBlock(
          // permission was successfully revoked
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(accountRevokeFromAccount(kAccount1,
                                       kAccount1Keypair,
                                       kAccount2,
                                       permissions::Grantable::kSetMyQuorum))
      .checkBlock(
          // permission cannot be revoked twice
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * Revoke without permission
 * @given ITF instance, two accounts, at the beginning first account have
 * can_grant permission and grants a permission to second account, then the
 * first account lost can_grant permission
 * @when the first account tries to revoke permission from the second
 * @then stateful validation fails
 */
/**
 * TODO igor-egorov, 2018-08-03, enable test case
 * https://soramitsu.atlassian.net/browse/IR-1572
 */
TEST_F(GrantPermissionFixture, DISABLED_RevokeWithoutPermission) {
  auto detach_role_tx =
      GrantPermissionFixture::TxBuilder()
          .createdTime(getUniqueTime())
          .creatorAccountId(IntegrationTestFramework::kAdminId)
          .quorum(1)
          .detachRole(kAccount1 + "@" + kDomain, kRole1)
          .build()
          .signAndAddSignature(kAdminKeypair)
          .finish();

  IntegrationTestFramework itf(1);
  createTwoAccounts(itf, {Role::kSetMyQuorum}, {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
                                    kAccount1Keypair,
                                    kAccount2,
                                    permissions::Grantable::kSetMyQuorum))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(detach_role_tx)
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(accountRevokeFromAccount(kAccount1,
                                       kAccount1Keypair,
                                       kAccount2,
                                       permissions::Grantable::kSetMyQuorum))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

namespace grantables {

  using gpf = GrantPermissionFixture;

  template <typename GrantableType>
  class GrantRevokeFixture : public GrantPermissionFixture {
   public:
    GrantableType grantable_type_;
  };

  struct GrantableType {
    const Role can_grant_permission_;
    const Grantable grantable_permission_;
    interface::types::HashType tx_hash_;

    virtual IntegrationTestFramework &prepare(GrantPermissionFixture &fixture,
                                              IntegrationTestFramework &itf) {
      (void)fixture;  // prevent warning about unused param, fixture is needed
                      // in some derivatives
      return itf;
    }

    virtual proto::Transaction testTransaction(
        GrantPermissionFixture &fixture) = 0;

   protected:
    GrantableType(const Role &can_grant_permission,
                  const Grantable &grantable_permission)
        : can_grant_permission_(can_grant_permission),
          grantable_permission_(grantable_permission) {}
  };

  struct AddMySignatory : public GrantableType {
    AddMySignatory()
        : GrantableType(Role::kAddMySignatory, Grantable::kAddMySignatory) {}

    proto::Transaction testTransaction(GrantPermissionFixture &f) override {
      return f.permiteeModifySignatory(
          &TestUnsignedTransactionBuilder::addSignatory,
          f.kAccount2,
          f.kAccount2Keypair,
          f.kAccount1);
    }
  };

  struct RemoveMySignatory : public GrantableType {
    RemoveMySignatory()
        : GrantableType(Role::kRemoveMySignatory,
                        Grantable::kRemoveMySignatory) {}

    IntegrationTestFramework &prepare(GrantPermissionFixture &f,
                                      IntegrationTestFramework &itf) override {
      auto account_id = f.kAccount1 + "@" + f.kDomain;
      auto add_signatory_tx =
          GrantPermissionFixture::TxBuilder()
              .createdTime(f.getUniqueTime())
              .creatorAccountId(account_id)
              .quorum(1)
              .addSignatory(account_id, f.kAccount2Keypair.publicKey())
              .build()
              .signAndAddSignature(f.kAccount1Keypair)
              .finish();
      itf.sendTx(add_signatory_tx)
          .checkProposal(
              [](auto &prop) { ASSERT_EQ(prop->transactions().size(), 1); })
          .checkBlock(
              [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
      return itf;
    }

    proto::Transaction testTransaction(GrantPermissionFixture &f) override {
      return f.permiteeModifySignatory(
          &TestUnsignedTransactionBuilder::removeSignatory,
          f.kAccount2,
          f.kAccount2Keypair,
          f.kAccount1);
    }
  };

  struct SetMyAccountDetail : public GrantableType {
    SetMyAccountDetail()
        : GrantableType(Role::kSetMyAccountDetail,
                        Grantable::kSetMyAccountDetail) {}

    proto::Transaction testTransaction(GrantPermissionFixture &f) override {
      return f.permiteeSetAccountDetail(f.kAccount2,
                                        f.kAccount2Keypair,
                                        f.kAccount1,
                                        f.kAccountDetailKey,
                                        f.kAccountDetailValue);
    }
  };

  struct SetMyQuorum : public GrantableType {
    SetMyQuorum()
        : GrantableType(Role::kSetMyQuorum, Grantable::kSetMyQuorum) {}

    proto::Transaction testTransaction(GrantPermissionFixture &f) override {
      return f.permiteeSetQuorum(
          f.kAccount2, f.kAccount2Keypair, f.kAccount1, 1);
    }
  };

  struct TransferMyAssets : public GrantableType {
    TransferMyAssets()
        : GrantableType(Role::kTransferMyAssets, Grantable::kTransferMyAssets) {
    }

    IntegrationTestFramework &prepare(GrantPermissionFixture &f,
                                      IntegrationTestFramework &itf) {
      auto create_and_transfer_coins =
          GrantPermissionFixture::TxBuilder()
              .createdTime(f.getUniqueTime())
              .creatorAccountId(itf.kAdminId)
              .quorum(1)
              .addAssetQuantity(f.kAsset, "9000.0")
              .transferAsset(itf.kAdminId,
                             f.kAccount1 + "@" + f.kDomain,
                             f.kAsset,
                             "init top up",
                             "8000.0")
              .build()
              .signAndAddSignature(f.kAdminKeypair)
              .finish();
      itf.sendTx(create_and_transfer_coins)
          .checkProposal(
              [](auto &prop) { ASSERT_EQ(prop->transactions().size(), 1); })
          .checkBlock(
              [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
      return itf;
    }

    proto::Transaction testTransaction(GrantPermissionFixture &f) override {
      return f.transferAssetFromSource(
          f.kAccount2, f.kAccount2Keypair, f.kAccount1, "1000.0", f.kAccount2);
    }
  };

  using GrantablePermissionsTypes = ::testing::Types<AddMySignatory,
                                                     RemoveMySignatory,
                                                     SetMyAccountDetail,
                                                     SetMyQuorum,
                                                     TransferMyAssets>;

  TYPED_TEST_CASE(GrantRevokeFixture, GrantablePermissionsTypes);

  TYPED_TEST(GrantRevokeFixture, GrantAndRevokePermission) {
    IntegrationTestFramework itf(1);

    gpf::createTwoAccounts(itf,
                           {this->grantable_type_.can_grant_permission_,
                            Role::kAddSignatory,
                            Role::kReceive},
                           {Role::kReceive})
        .sendTx(gpf::accountGrantToAccount(
            gpf::kAccount1,
            gpf::kAccount1Keypair,
            gpf::kAccount2,
            this->grantable_type_.grantable_permission_))
        .skipProposal()
        .checkBlock(
            // permission was successfully granted
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
    this->grantable_type_.prepare(*this, itf)
        .sendTx(this->grantable_type_.testTransaction(*this))
        .checkProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
        .sendTx(gpf::accountRevokeFromAccount(
            gpf::kAccount1,
            gpf::kAccount1Keypair,
            gpf::kAccount2,
            this->grantable_type_.grantable_permission_))
        .skipProposal()
        .checkBlock(
            // permission was successfully revoked
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
    auto last_check_tx = this->grantable_type_.testTransaction(*this);
    std::vector<interface::types::HashType> hashes{last_check_tx.hash()};
    auto last_tx_status_query = TestUnsignedQueryBuilder()
                                    .creatorAccountId(itf.kAdminId)
                                    .createdTime(this->getUniqueTime())
                                    .queryCounter(1)
                                    .getTransactions(hashes)
                                    .build()
                                    .signAndAddSignature(this->kAdminKeypair)
                                    .finish();
    itf.sendTx(last_check_tx)
        .checkProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkBlock(
            [](auto &block) { EXPECT_EQ(block->transactions().size(), 0); })
        .getTxStatus(
            last_check_tx.hash(),
            [](auto &status) {
              auto message = status.errorMessage();

              ASSERT_NE(
                  message.find("has permission command validation failed"),
                  std::string::npos)
                  << "Fail reason: " << message;
            })
        .done();
  }

}  // namespace grantables
