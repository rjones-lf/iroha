/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/grantable_permissions_fixture.hpp"

#include "builders/protobuf/builder_templates/query_template.hpp"

using namespace integration_framework;

using namespace shared_model;
using namespace shared_model::interface;
using namespace shared_model::interface::permissions;

/**
 * C269 Revoke permission from a non-existing account
 * @given ITF instance and only one account with can_grant permission
 * @when the account tries to revoke grantable permission from non-existing
 * account
 * @then transaction would not be committed
 */
TEST_F(GrantablePermissionsFixture, RevokeFromNonExistingAccount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeAccountWithPerms(
          kAccount1, kAccount1Keypair, {Role::kSetMyQuorum}, kRole1))
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTx(revokePermission(kAccount1,
                               kAccount1Keypair,
                               kAccount2,
                               permissions::Grantable::kSetMyQuorum))
      .checkProposal(
          // transaction is stateless valid
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkVerifiedProposal(
          // transaction is not stateful valid (kAccount2 does not exist)
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); });
}

/**
 * C271 Revoke permission more than once
 * @given ITF instance, two accounts, the first account has granted a permission
 * to the second
 * @when the first account revokes the permission twice
 * @then the second revoke does not pass stateful validation
 */
TEST_F(GrantablePermissionsFixture, RevokeTwice) {
  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair);
  createTwoAccounts(itf, {Role::kSetMyQuorum}, {Role::kReceive})
      .sendTx(grantPermission(kAccount1,
                              kAccount1Keypair,
                              kAccount2,
                              permissions::Grantable::kSetMyQuorum))
      .skipVerifiedProposal()
      .checkBlock(
          // permission was successfully granted
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTxAwait(
          revokePermission(
              kAccount1,
              kAccount1Keypair,
              kAccount2,
              permissions::Grantable::kSetMyQuorum),  // permission was
                                                      // successfully revoked
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(revokePermission(kAccount1,
                               kAccount1Keypair,
                               kAccount2,
                               permissions::Grantable::kSetMyQuorum))
      .checkVerifiedProposal(
          // permission cannot be revoked twice
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); });
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
TEST_F(GrantablePermissionsFixture, DISABLED_RevokeWithoutPermission) {
  auto detach_role_tx =
      GrantablePermissionsFixture::TxBuilder()
          .createdTime(getUniqueTime())
          .creatorAccountId(IntegrationTestFramework::kAdminId)
          .quorum(1)
          .detachRole(kAccount1 + "@" + kDomain, kRole1)
          .build()
          .signAndAddSignature(kAdminKeypair)
          .finish();

  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair);
  createTwoAccounts(itf, {Role::kSetMyQuorum}, {Role::kReceive})
      .sendTxAwait(
          grantPermission(kAccount1,
                          kAccount1Keypair,
                          kAccount2,
                          permissions::Grantable::kSetMyQuorum),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTxAwait(
          detach_role_tx,
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(revokePermission(kAccount1,
                               kAccount1Keypair,
                               kAccount2,
                               permissions::Grantable::kSetMyQuorum))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 0); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

namespace grantables {

  using gpf = GrantablePermissionsFixture;

  template <typename GrantableType>
  class GrantRevokeFixture : public GrantablePermissionsFixture {
   public:
    GrantableType grantable_type_;
  };

  struct GrantableType {
    const Role can_grant_permission_;
    const Grantable grantable_permission_;
    interface::types::HashType tx_hash_;

    virtual IntegrationTestFramework &prepare(
        GrantablePermissionsFixture &fixture, IntegrationTestFramework &itf) {
      (void)fixture;  // prevent warning about unused param, fixture is needed
                      // in some derivatives
      return itf;
    }

    virtual proto::Transaction testTransaction(
        GrantablePermissionsFixture &fixture) = 0;

   protected:
    GrantableType(const Role &can_grant_permission,
                  const Grantable &grantable_permission)
        : can_grant_permission_(can_grant_permission),
          grantable_permission_(grantable_permission) {}
  };

  struct AddMySignatory : public GrantableType {
    AddMySignatory()
        : GrantableType(Role::kAddMySignatory, Grantable::kAddMySignatory) {}

    proto::Transaction testTransaction(
        GrantablePermissionsFixture &f) override {
      return f.permitteeModifySignatory(
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

    IntegrationTestFramework &prepare(GrantablePermissionsFixture &f,
                                      IntegrationTestFramework &itf) override {
      auto account_id = f.kAccount1 + "@" + f.kDomain;
      auto pkey =
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
              .publicKey();
      auto add_signatory_tx =
          GrantablePermissionsFixture::TxBuilder()
              .createdTime(f.getUniqueTime())
              .creatorAccountId(account_id)
              .quorum(1)
              .addSignatory(account_id, f.kAccount2Keypair.publicKey())
              .addSignatory(account_id, pkey)
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

    proto::Transaction testTransaction(
        GrantablePermissionsFixture &f) override {
      return f.permitteeModifySignatory(
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

    proto::Transaction testTransaction(
        GrantablePermissionsFixture &f) override {
      return f.setAccountDetail(f.kAccount2,
                                f.kAccount2Keypair,
                                f.kAccount1,
                                f.kAccountDetailKey,
                                f.kAccountDetailValue);
    }
  };

  struct SetMyQuorum : public GrantableType {
    SetMyQuorum()
        : GrantableType(Role::kSetMyQuorum, Grantable::kSetMyQuorum) {}

    proto::Transaction testTransaction(
        GrantablePermissionsFixture &f) override {
      return f.setQuorum(f.kAccount2, f.kAccount2Keypair, f.kAccount1, 1);
    }
  };

  struct TransferMyAssets : public GrantableType {
    TransferMyAssets()
        : GrantableType(Role::kTransferMyAssets, Grantable::kTransferMyAssets) {
    }

    IntegrationTestFramework &prepare(GrantablePermissionsFixture &f,
                                      IntegrationTestFramework &itf) {
      auto create_and_transfer_coins =
          GrantablePermissionsFixture::TxBuilder()
              .createdTime(f.getUniqueTime())
              .creatorAccountId(itf.kAdminId)
              .quorum(1)
              .addAssetQuantity(f.kAssetId, "9000.0")
              .transferAsset(itf.kAdminId,
                             f.kAccount1 + "@" + f.kDomain,
                             f.kAssetId,
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

    proto::Transaction testTransaction(
        GrantablePermissionsFixture &f) override {
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

  /**
   * The test iterates over helper types (GrantablePermissionsTypes).
   * That helper types contain information about required Grantable and Role
   * permissions for the test.
   *
   * The test does the following:
   * - creates two accounts
   * - first account grants permission to the second account
   * - does preparation of the first account (for example gives assets for
   * future transfer during the main part of the test)
   * - does the test transaction (each GrantablePermissionType has own test
   * transaction)
   * - first account revokes the permission from the second account
   * - does the test transaction again
   * - checks that the last transaction was failed due to a missing permission
   */
  TYPED_TEST(GrantRevokeFixture, GrantAndRevokePermission) {
    IntegrationTestFramework itf(1);
    itf.setInitialState(gpf::kAdminKeypair);

    gpf::createTwoAccounts(itf,
                           {this->grantable_type_.can_grant_permission_,
                            Role::kAddSignatory,
                            Role::kReceive},
                           {Role::kReceive})
        .sendTxAwait(
            gpf::grantPermission(gpf::kAccount1,
                                 gpf::kAccount1Keypair,
                                 gpf::kAccount2,
                                 this->grantable_type_.grantable_permission_),
            // permission was successfully granted
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
    this->grantable_type_.prepare(*this, itf)
        .sendTx(this->grantable_type_.testTransaction(*this))
        .checkProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .skipVerifiedProposal()
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
        .sendTxAwait(
            gpf::revokePermission(gpf::kAccount1,
                                  gpf::kAccount1Keypair,
                                  gpf::kAccount2,
                                  this->grantable_type_.grantable_permission_),
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
        .skipVerifiedProposal()
        .skipBlock()
        .getTxStatus(last_check_tx.hash(),
                     [](auto &status) {
                       auto message = status.errorMessage();

                       ASSERT_NE(message.find("did not pass verification"),
                                 std::string::npos)
                           << "Fail reason: " << message
                           << "\nRaw status:" << status.toString();
                       // we saw empty message was received once
                       // that is why we have added the raw print of status
                     })
        .done();
  }

}  // namespace grantables
