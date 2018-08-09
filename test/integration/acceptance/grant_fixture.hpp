/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_GRANT_FIXTURE_H
#define IROHA_GRANT_FIXTURE_H

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

class GrantPermissionFixture : public AcceptanceFixture {
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
   * Forms a transaction such that creator of transaction revokes a permission
   * from permittee
   * @param cerator_account_name - transaction creator account name without
   * domain
   * @param creator_key - key of transaction creator
   * @param permittee_account_name - account to revoke permission from
   * @param revoke_permission - grantable permission to be revoked
   * @return
   */
  proto::Transaction accountRevokeFromAccount(
      const types::AccountNameType &creator_account_name,
      const crypto::Keypair &creator_key,
      const types::AccountNameType &permittee_account_name,
      const interface::permissions::Grantable &revoke_permission) {
    const types::AccountIdType creator_account_id =
        creator_account_name + "@" + kDomain;
    const types::AccountIdType permittee_account_id =
        permittee_account_name + "@" + kDomain;
    return TxBuilder()
        .creatorAccountId(creator_account_id)
        .createdTime(getUniqueTime())
        .revokePermission(permittee_account_id, revoke_permission)
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

#endif  // IROHA_GRANT_FIXTURE_H
