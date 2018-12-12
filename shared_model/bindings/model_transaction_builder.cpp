/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bindings/model_transaction_builder.hpp"

namespace shared_model {
  namespace bindings {
    ModelTransactionBuilder::ModelTransactionBuilder() {
      *this = creatorAccountId("").createdTime(0).quorum(1);
    }

    ModelTransactionBuilder ModelTransactionBuilder::creatorAccountId(
        const interface::types::AccountIdType &account_id) {
      return ModelTransactionBuilder(builder_.creatorAccountId(account_id));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createdTime(
        interface::types::TimestampType created_time) {
      return ModelTransactionBuilder(builder_.createdTime(created_time));
    }

    ModelTransactionBuilder ModelTransactionBuilder::quorum(
        interface::types::QuorumType quorum) {
      return ModelTransactionBuilder(builder_.quorum(quorum));
    }

    ModelTransactionBuilder ModelTransactionBuilder::batchMeta(
        interface::types::BatchType type,
        const std::vector<interface::types::HashType> &hashes) {
      return ModelTransactionBuilder(builder_.batchMeta(type, hashes));
    }

    ModelTransactionBuilder ModelTransactionBuilder::addAssetQuantity(
        const interface::types::AssetIdType &asset_id,
        const std::string &amount) {
      return ModelTransactionBuilder(
          builder_.addAssetQuantity(asset_id, amount));
    }

    ModelTransactionBuilder ModelTransactionBuilder::addPeer(
        const interface::types::AddressType &address,
        const interface::types::PubkeyType &peer_key) {
      return ModelTransactionBuilder(builder_.addPeer(address, peer_key));
    }

    ModelTransactionBuilder ModelTransactionBuilder::addSignatory(
        const interface::types::AddressType &account_id,
        const interface::types::PubkeyType &public_key) {
      return ModelTransactionBuilder(
          builder_.addSignatory(account_id, public_key));
    }

    ModelTransactionBuilder ModelTransactionBuilder::removeSignatory(
        const interface::types::AddressType &account_id,
        const interface::types::PubkeyType &public_key) {
      return ModelTransactionBuilder(
          builder_.removeSignatory(account_id, public_key));
    }

    ModelTransactionBuilder ModelTransactionBuilder::appendRole(
        const interface::types::AccountIdType &account_id,
        const interface::types::RoleIdType &role_name) {
      return ModelTransactionBuilder(
          builder_.appendRole(account_id, role_name));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createAsset(
        const interface::types::AssetNameType &asset_name,
        const interface::types::DomainIdType &domain_id,
        interface::types::PrecisionType precision) {
      return ModelTransactionBuilder(
          builder_.createAsset(asset_name, domain_id, precision));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createAccount(
        const interface::types::AccountNameType &account_name,
        const interface::types::AddressType &domain_id,
        const interface::types::PubkeyType &main_pubkey) {
      return ModelTransactionBuilder(
          builder_.createAccount(account_name, domain_id, main_pubkey));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createDomain(
        const interface::types::AddressType &domain_id,
        const interface::types::RoleIdType &default_role) {
      return ModelTransactionBuilder(
          builder_.createDomain(domain_id, default_role));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createRole(
        const interface::types::RoleIdType &role_name,
        const interface::RolePermissionSet &permissions) {
      return ModelTransactionBuilder(
          builder_.createRole(role_name, permissions));
    }

    ModelTransactionBuilder ModelTransactionBuilder::detachRole(
        const interface::types::AccountIdType &account_id,
        const interface::types::RoleIdType &role_name) {
      return ModelTransactionBuilder(
          builder_.detachRole(account_id, role_name));
    }

    ModelTransactionBuilder ModelTransactionBuilder::grantPermission(
        const interface::types::AccountIdType &account_id,
        interface::permissions::Grantable permission) {
      return ModelTransactionBuilder(
          builder_.grantPermission(account_id, permission));
    }

    ModelTransactionBuilder ModelTransactionBuilder::revokePermission(
        const interface::types::AccountIdType &account_id,
        interface::permissions::Grantable permission) {
      return ModelTransactionBuilder(
          builder_.revokePermission(account_id, permission));
    }

    ModelTransactionBuilder ModelTransactionBuilder::setAccountDetail(
        const interface::types::AccountIdType &account_id,
        const interface::types::AccountDetailKeyType &key,
        const interface::types::AccountDetailValueType &value) {
      return ModelTransactionBuilder(
          builder_.setAccountDetail(account_id, key, value));
    }

    ModelTransactionBuilder ModelTransactionBuilder::setAccountQuorum(
        const interface::types::AddressType &account_id,
        interface::types::QuorumType quorum) {
      return ModelTransactionBuilder(
          builder_.setAccountQuorum(account_id, quorum));
    }

    ModelTransactionBuilder ModelTransactionBuilder::subtractAssetQuantity(
        const interface::types::AssetIdType &asset_id,
        const std::string &amount) {
      return ModelTransactionBuilder(
          builder_.subtractAssetQuantity(asset_id, amount));
    }

    ModelTransactionBuilder ModelTransactionBuilder::transferAsset(
        const interface::types::AccountIdType &src_account_id,
        const interface::types::AccountIdType &dest_account_id,
        const interface::types::AssetIdType &asset_id,
        const std::string &description,
        const std::string &amount) {
      return ModelTransactionBuilder(builder_.transferAsset(
          src_account_id, dest_account_id, asset_id, description, amount));
    }

    proto::UnsignedWrapper<proto::Transaction>
    ModelTransactionBuilder::build() {
      return builder_.build();
    }
  }  // namespace bindings
}  // namespace shared_model
