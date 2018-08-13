/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/grantable_permissions_fixture.hpp"

using namespace shared_model::interface::permissions;

shared_model::proto::Transaction
GrantablePermissionsFixture::makeAccountWithPerms(
    const shared_model::interface::types::AccountNameType &user,
    const shared_model::crypto::Keypair &key,
    const shared_model::interface::RolePermissionSet &perms,
    const shared_model::interface::types::RoleIdType &role) {
  return createUserWithPerms(user, key.publicKey(), role, perms)
      .build()
      .signAndAddSignature(kAdminKeypair)
      .finish();
}

integration_framework::IntegrationTestFramework &
GrantablePermissionsFixture::createTwoAccounts(
    integration_framework::IntegrationTestFramework &itf,
    const shared_model::interface::RolePermissionSet &perm1,
    const shared_model::interface::RolePermissionSet &perm2) {
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeAccountWithPerms(kAccount1, kAccount1Keypair, perm1, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeAccountWithPerms(kAccount2, kAccount2Keypair, perm2, kRole2))
      .skipProposal()
      .skipBlock();
  return itf;
}

shared_model::proto::Transaction
GrantablePermissionsFixture::accountGrantToAccount(
    const shared_model::interface::types::AccountNameType &creator_account_name,
    const shared_model::crypto::Keypair &creator_key,
    const shared_model::interface::types::AccountNameType
        &permitee_account_name,
    const shared_model::interface::permissions::Grantable &grant_permission) {
  const auto creator_account_id = creator_account_name + "@" + kDomain;
  const auto permitee_account_id = permitee_account_name + "@" + kDomain;
  return TxBuilder()
      .creatorAccountId(creator_account_id)
      .createdTime(getUniqueTime())
      .grantPermission(permitee_account_id, grant_permission)
      .quorum(1)
      .build()
      .signAndAddSignature(creator_key)
      .finish();
}

shared_model::proto::Transaction
GrantablePermissionsFixture::accountRevokeFromAccount(
    const shared_model::interface::types::AccountNameType &creator_account_name,
    const shared_model::crypto::Keypair &creator_key,
    const shared_model::interface::types::AccountNameType
        &permittee_account_name,
    const shared_model::interface::permissions::Grantable &revoke_permission) {
  const auto creator_account_id = creator_account_name + "@" + kDomain;
  const auto permittee_account_id = permittee_account_name + "@" + kDomain;
  return TxBuilder()
      .creatorAccountId(creator_account_id)
      .createdTime(getUniqueTime())
      .revokePermission(permittee_account_id, revoke_permission)
      .quorum(1)
      .build()
      .signAndAddSignature(creator_key)
      .finish();
}

shared_model::proto::Transaction
GrantablePermissionsFixture::permiteeModifySignatory(
    GrantablePermissionsFixture::TxBuilder (
        GrantablePermissionsFixture::TxBuilder::*f)(
        const shared_model::interface::types::AccountIdType &,
        const shared_model::interface::types::PubkeyType &) const,
    const shared_model::interface::types::AccountNameType
        &permitee_account_name,
    const shared_model::crypto::Keypair &permitee_key,
    const shared_model::interface::types::AccountNameType &account_name) {
  const auto permitee_account_id = permitee_account_name + "@" + kDomain;
  const auto account_id = account_name + "@" + kDomain;
  return (TxBuilder()
              .creatorAccountId(permitee_account_id)
              .createdTime(getUniqueTime())
          .*f)(account_id, permitee_key.publicKey())
      .quorum(1)
      .build()
      .signAndAddSignature(permitee_key)
      .finish();
}

shared_model::proto::Transaction GrantablePermissionsFixture::permiteeSetQuorum(
    const shared_model::interface::types::AccountNameType
        &permitee_account_name,
    const shared_model::crypto::Keypair &permitee_key,
    const shared_model::interface::types::AccountNameType &account_name,
    shared_model::interface::types::QuorumType quorum) {
  const auto permitee_account_id = permitee_account_name + "@" + kDomain;
  const auto account_id = account_name + "@" + kDomain;
  return TxBuilder()
      .creatorAccountId(permitee_account_id)
      .createdTime(getUniqueTime())
      .setAccountQuorum(account_id, quorum)
      .quorum(1)
      .build()
      .signAndAddSignature(permitee_key)
      .finish();
}

shared_model::proto::Transaction
GrantablePermissionsFixture::permiteeSetAccountDetail(
    const shared_model::interface::types::AccountNameType
        &permitee_account_name,
    const shared_model::crypto::Keypair &permitee_key,
    const shared_model::interface::types::AccountNameType &account_name,
    const shared_model::interface::types::AccountDetailKeyType &key,
    const shared_model::interface::types::AccountDetailValueType &detail) {
  const auto permitee_account_id = permitee_account_name + "@" + kDomain;
  const auto account_id = account_name + "@" + kDomain;
  return TxBuilder()
      .creatorAccountId(permitee_account_id)
      .createdTime(getUniqueTime())
      .setAccountDetail(account_id, key, detail)
      .quorum(1)
      .build()
      .signAndAddSignature(permitee_key)
      .finish();
}

shared_model::proto::Transaction
GrantablePermissionsFixture::addAssetAndTransfer(
    const shared_model::interface::types::AccountNameType &creator_name,
    const shared_model::crypto::Keypair &creator_key,
    const shared_model::interface::types::AccountNameType &amount,
    const shared_model::interface::types::AccountNameType &receiver_name) {
  const auto creator_account_id = creator_name + "@" + kDomain;
  const auto receiver_account_id = receiver_name + "@" + kDomain;
  const auto asset_id =
      integration_framework::IntegrationTestFramework::kAssetName + "#"
      + kDomain;
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

shared_model::proto::Transaction
GrantablePermissionsFixture::transferAssetFromSource(
    const shared_model::interface::types::AccountNameType &creator_name,
    const shared_model::crypto::Keypair &creator_key,
    const shared_model::interface::types::AccountNameType &source_account_name,
    const std::string &amount,
    const shared_model::interface::types::AccountNameType &receiver_name) {
  const auto creator_account_id = creator_name + "@" + kDomain;
  const auto source_account_id = source_account_name + "@" + kDomain;
  const auto receiver_account_id = receiver_name + "@" + kDomain;
  const auto asset_id =
      integration_framework::IntegrationTestFramework::kAssetName + "#"
      + kDomain;
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

shared_model::proto::Query GrantablePermissionsFixture::querySignatories(
    const shared_model::interface::types::AccountNameType &account_name,
    const shared_model::crypto::Keypair &account_key) {
  const std::string account_id = account_name + "@" + kDomain;
  return shared_model::proto::QueryBuilder()
      .creatorAccountId(account_id)
      .createdTime(getUniqueTime())
      .queryCounter(1)
      .getSignatories(account_id)
      .build()
      .signAndAddSignature(account_key)
      .finish();
}

shared_model::proto::Query GrantablePermissionsFixture::queryAccount(
    const shared_model::interface::types::AccountNameType &account_name,
    const shared_model::crypto::Keypair &account_key) {
  const auto account_id = account_name + "@" + kDomain;
  return shared_model::proto::QueryBuilder()
      .creatorAccountId(account_id)
      .createdTime(getUniqueTime())
      .queryCounter(1)
      .getAccount(account_id)
      .build()
      .signAndAddSignature(account_key)
      .finish();
}

shared_model::proto::Query GrantablePermissionsFixture::queryAccountDetail(
    const shared_model::interface::types::AccountNameType &account_name,
    const shared_model::crypto::Keypair &account_key) {
  const auto account_id = account_name + "@" + kDomain;
  return shared_model::proto::QueryBuilder()
      .creatorAccountId(account_id)
      .createdTime(getUniqueTime())
      .queryCounter(1)
      .getAccountDetail(account_id)
      .build()
      .signAndAddSignature(account_key)
      .finish();
}