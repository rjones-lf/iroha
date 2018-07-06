/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;

using namespace shared_model;
using namespace shared_model::interface;
using namespace shared_model::interface::permissions;

class GrantPermissionTest : public AcceptanceFixture {
 public:
  using TxBuilder = TestUnsignedTransactionBuilder;

  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeAccountWithPerms(
      const std::string &user,
      const crypto::Keypair &key,
      const shared_model::interface::RolePermissionSet &perms,
      const std::string &role) {
    return createUserWithPerms(user, key.publicKey(), role, perms)
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  /**
   * Creates two accounts with corresponding permission sets
   * @param perm1 set of permissions for account #1
   * @param perm2 set of permissions for account #2
   * @return reference to ITF object with two transactions
   */
  decltype(auto) createTwoAccounts(
      IntegrationTestFramework &itf,
      const shared_model::interface::RolePermissionSet &perm1,
      const shared_model::interface::RolePermissionSet &perm2) {
    itf.setInitialState(kAdminKeypair)
        .sendTx(
            makeAccountWithPerms(kAccount1, kAccount1Keypair, perm1, kRole1))
        .skipProposal()
        .skipBlock()
        .sendTx(
            makeAccountWithPerms(kAccount2, kAccount2Keypair, perm2, kRole2))
        .skipProposal()
        .skipBlock();
    return itf;
  }

  /**
   * Forms a transaction such that creator of transaction
   * grants permittee a permission
   * @param creatorAccountName — a name, e.g. user
   * @param permitteeAccountId — an id of grantee, most likely name@test
   * @param grantPermission — any permission from the set of grantable
   * @return a Transaction object
   */
  proto::Transaction accountGrantToAccount(
      const std::string &creator_account_name,
      const crypto::Keypair &creator_key,
      const std::string &permitee_account_name,
      const interface::permissions::Grantable &grant_permission) {
    const std::string creator_account_id = creator_account_name + "@" + kDomain;
    const std::string permitee_account_id =
        permitee_account_name + "@" + kDomain;
    return TxBuilder()
        .creatorAccountId(creator_account_id)
        .createdTime(getUniqueTime())
        .grantPermission(permitee_account_id, grant_permission)
        .quorum(1)
        .build()
        .signAndAddSignature(creator_key)
        .finish();
  }

  /**
   * Forms a transaction that either adds or removes signatory of an account
   * @param f Add or Remove signatory function
   * @param permitee_account_name name of account which is granted permission
   * @param permitee_key key of account which is granted permission
   * @param account_name account name which has granted permission to permitee
   * @return a transaction
   */
  proto::Transaction permiteeModifySignatory(
      TxBuilder (TxBuilder::*f)(const interface::types::AccountIdType &,
                                const interface::types::PubkeyType &) const,
      const std::string &permitee_account_name,
      const crypto::Keypair &permitee_key,
      const std::string &account_name) {
    const std::string permitee_account_id =
        permitee_account_name + "@" + kDomain;
    const std::string account_id = account_name + "@" + kDomain;
    return (TxBuilder()
                .creatorAccountId(permitee_account_id)
                .createdTime(getUniqueTime())
            .*f)(account_id, permitee_key.publicKey())
        .quorum(1)
        .build()
        .signAndAddSignature(permitee_key)
        .finish();
  }

  /**
   * Forms a transaction that allows permitted user to modify quorum field
   * @param permitee_account_name name of account which is granted permission
   * @param permitee_key key of account which is granted permission
   * @param account_name account name which has granted permission to permitee
   * @param quorum quorum field
   * @return a transaction
   */
  proto::Transaction permiteeSetQuorum(const std::string &permitee_account_name,
                                       const crypto::Keypair &permitee_key,
                                       const std::string &account_name,
                                       int quorum) {
    const std::string permitee_account_id =
        permitee_account_name + "@" + kDomain;
    const std::string account_id = account_name + "@" + kDomain;
    return TxBuilder()
        .creatorAccountId(permitee_account_id)
        .createdTime(getUniqueTime())
        .setAccountQuorum(account_id, quorum)
        .quorum(1)
        .build()
        .signAndAddSignature(permitee_key)
        .finish();
  }

  /**
   * Forms a transaction that allows permitted user to set details of the
   * account
   * @param permitee_account_name name of account which is granted permission
   * @param permitee_key key of account which is granted permission
   * @param account_name account name which has granted permission to permitee
   * @param key of the data to set
   * @param detail is the data value
   * @return a transaction
   */
  proto::Transaction permiteeSetAccountDetail(
      const std::string &permitee_account_name,
      const crypto::Keypair &permitee_key,
      const std::string &account_name,
      const std::string &key,
      const std::string &detail) {
    const std::string permitee_account_id =
        permitee_account_name + "@" + kDomain;
    const std::string account_id = account_name + "@" + kDomain;
    return TxBuilder()
        .creatorAccountId(permitee_account_id)
        .createdTime(getUniqueTime())
        .setAccountDetail(account_id, key, detail)
        .quorum(1)
        .build()
        .signAndAddSignature(permitee_key)
        .finish();
  }

  /**
   * Adds specified amount of an asset and transfers it
   * @param creator_name account name which is creating transfer transaction
   * @param creator_key account key which is creating transfer transaction
   * @param amount created amount of a default asset in AcceptanceFixture
   * @param receiver_name name of an account which receives transfer
   * @return a transaction
   */
  proto::Transaction addAssetAndTransfer(const std::string &creator_name,
                                         const crypto::Keypair &creator_key,
                                         const std::string &amount,
                                         const std::string &receiver_name) {
    const std::string creator_account_id = creator_name + "@" + kDomain;
    const std::string receiver_account_id = receiver_name + "@" + kDomain;
    const std::string asset_id =
        IntegrationTestFramework::kAssetName + "#" + kDomain;
    return TxBuilder()
        .creatorAccountId(creator_account_id)
        .createdTime(getUniqueTime())
        .addAssetQuantity(asset_id, amount)
        .transferAsset(
            creator_account_id, receiver_account_id, asset_id, "", amount)
        .quorum(1)
        .build()
        .signAndAddSignature(creator_key)
        .finish();
  }

  /**
   * Transaction, that transfers standard asset from source account to receiver
   * @param creator_name account name which is creating transfer transaction
   * @param creator_key account key which is creating transfer transaction
   * @param source_account_name account which has assets to transfer
   * @param amount amount of transfered asset
   * @param receiver_name name of an account which receives transfer
   * @return a transaction
   */
  proto::Transaction transferAssetFromSource(
      const std::string &creator_name,
      const crypto::Keypair &creator_key,
      const std::string &source_account_name,
      const std::string &amount,
      const std::string &receiver_name) {
    const std::string creator_account_id = creator_name + "@" + kDomain;
    const std::string source_account_id = source_account_name + "@" + kDomain;
    const std::string receiver_account_id = receiver_name + "@" + kDomain;
    const std::string asset_id =
        IntegrationTestFramework::kAssetName + "#" + kDomain;
    return TxBuilder()
        .creatorAccountId(creator_account_id)
        .createdTime(getUniqueTime())
        .transferAsset(
            source_account_id, receiver_account_id, asset_id, "", amount)
        .quorum(1)
        .build()
        .signAndAddSignature(creator_key)
        .finish();
  }

  /**
   * Get signatories of an account (same as transaction creator)
   * @param account_name account name which has signatories
   * @param account_key account key which has signatories
   * @return
   */
  proto::Query querySignatories(const std::string &account_name,
                                const crypto::Keypair &account_key) {
    const std::string account_id = account_name + "@" + kDomain;
    return proto::QueryBuilder()
        .creatorAccountId(account_id)
        .createdTime(getUniqueTime())
        .queryCounter(1)
        .getSignatories(account_id)
        .build()
        .signAndAddSignature(account_key)
        .finish();
  }

  /**
   * Get account metadata in order to check quorum field
   * @param account_name account name
   * @param account_key account key
   * @return a query
   */
  proto::Query queryAccount(const std::string &account_name,
                            const crypto::Keypair &account_key) {
    const std::string account_id = account_name + "@" + kDomain;
    return proto::QueryBuilder()
        .creatorAccountId(account_id)
        .createdTime(getUniqueTime())
        .queryCounter(1)
        .getAccount(account_id)
        .build()
        .signAndAddSignature(account_key)
        .finish();
  }

  /**
   * Get account details
   * @param account_name account name which has AccountDetails in JSON
   * @param account_key account key which has AccountDetails in JSON
   * @return a query
   */
  proto::Query queryAccountDetail(const std::string &account_name,
                                  const crypto::Keypair &account_key) {
    const std::string account_id = account_name + "@" + kDomain;
    return proto::QueryBuilder()
        .creatorAccountId(account_id)
        .createdTime(getUniqueTime())
        .queryCounter(1)
        .getAccountDetail(account_id)
        .build()
        .signAndAddSignature(account_key)
        .finish();
  }

  /**
   * Creates a lambda that checks query response for signatures
   * @param signatory a keypair that has a public key to compare
   * @param quantity required quantity of signatories
   * @param is_contained true if the signtory is in the set
   * @return function
   */
  static auto checkSignatorySet(const crypto::Keypair &signatory,
                                int quantity,
                                bool is_contained) {
    return [&signatory, quantity, is_contained](
               const shared_model::proto::QueryResponse &query_response) {
      ASSERT_NO_THROW({
        const auto &resp = boost::apply_visitor(
            framework::SpecifiedVisitor<interface::SignatoriesResponse>(),
            query_response.get());

        ASSERT_EQ(resp.keys().size(), quantity);
        auto &keys = resp.keys();

        ASSERT_EQ((std::find(keys.begin(), keys.end(), signatory.publicKey())
                   != keys.end()),
                  is_contained);
      });
    };
  }

  /**
   * Lambda method that checks quorum to be equal to passed quantity value
   * @param quorum_quantity value of quorum that has to be equal in query
   * response
   * @return function
   */
  static auto checkQuorum(int quorum_quantity) {
    return [quorum_quantity](
               const shared_model::proto::QueryResponse &query_response) {
      ASSERT_NO_THROW({
        const auto &resp = boost::apply_visitor(
            framework::SpecifiedVisitor<interface::AccountResponse>(),
            query_response.get());

        ASSERT_EQ(resp.account().quorum(), quorum_quantity);
      });
    };
  }

  /**
   * Lambda method that checks account details to contain key and value (detail)
   * @param key key which has to be equal in account details
   * @param detail value which has to be equal in account details
   * @return function
   */
  static auto checkAccountDetail(const std::string &key,
                                 const std::string &detail) {
    return [&key,
            &detail](const shared_model::proto::QueryResponse &query_response) {
      ASSERT_NO_THROW({
        const auto &resp = boost::apply_visitor(
            framework::SpecifiedVisitor<interface::AccountDetailResponse>(),
            query_response.get());
        ASSERT_TRUE(resp.detail().find(key) != std::string::npos);
        ASSERT_TRUE(resp.detail().find(detail) != std::string::npos);
      });
    };
  }

  const std::string kAccount1 = "accountone";
  const std::string kAccount2 = "accounttwo";

  const std::string kRole1 = "roleone";
  const std::string kRole2 = "roletwo";

  const crypto::Keypair kAccount1Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kAccount2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();

  const std::string kAccountDetailKey = "some_key";
  const std::string kAccountDetailValue = "some_value";

  const RolePermissionSet kCanGrantAll{permissions::Role::kAddMySignatory,
                                       permissions::Role::kRemoveMySignatory,
                                       permissions::Role::kSetMyQuorum,
                                       permissions::Role::kSetMyAccountDetail,
                                       permissions::Role::kTransferMyAssets};

  const std::vector<permissions::Grantable> kAllGrantable{
      permissions::Grantable::kAddMySignatory,
      permissions::Grantable::kRemoveMySignatory,
      permissions::Grantable::kSetMyQuorum,
      permissions::Grantable::kSetMyAccountDetail,
      permissions::Grantable::kTransferMyAssets};
};

/**
 * C256 Grant permission to a non-existing account
 * @given an account with rights to grant rights to other accounts
 * @when the account grants rights to non-existing account
 * @then this transaction is stateful invalid
 * AND it is not written in the block
 */
TEST_F(GrantPermissionTest, GrantToInexistingAccount) {
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
TEST_F(GrantPermissionTest, GrantAddSignatoryPermission) {
  auto expected_number_of_signatories = 2;
  auto is_contained = true;
  auto check_if_signatory_is_contained = checkSignatorySet(
      kAccount2Keypair, expected_number_of_signatories, is_contained);

  IntegrationTestFramework itf(1);
  createTwoAccounts(
      itf, {Role::kAddMySignatory, Role::kGetMySignatories}, {Role::kReceive})
      .sendTx(accountGrantToAccount(kAccount1,
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
TEST_F(GrantPermissionTest, GrantRemoveSignatoryPermission) {
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
TEST_F(GrantPermissionTest, GrantSetQuorumPermission) {
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
TEST_F(GrantPermissionTest, GrantSetAccountDetailPermission) {
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
TEST_F(GrantPermissionTest, GrantTransferPermission) {
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
TEST_F(GrantPermissionTest, GrantWithoutGrantPermissions) {
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

TEST_F(GrantPermissionTest, GrantMoreThanOnce) {
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
