/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bindings/model_transaction_builder.hpp"

namespace shared_model {
  namespace bindings {
    ModelTransactionBuilder::ModelTransactionBuilder() {
      *this = creatorAccountId("").createdTime(0).txCounter(0);
    }

    ModelTransactionBuilder ModelTransactionBuilder::creatorAccountId(
        const interface::types::AccountIdType &account_id) {
      return ModelTransactionBuilder(builder_.creatorAccountId(account_id));
    }

    ModelTransactionBuilder ModelTransactionBuilder::txCounter(
        interface::types::CounterType tx_counter) {
      return ModelTransactionBuilder(builder_.txCounter(tx_counter));
    }

    ModelTransactionBuilder ModelTransactionBuilder::createdTime(
        interface::types::TimestampType created_time) {
      return ModelTransactionBuilder(builder_.createdTime(created_time));
    }

    ModelTransactionBuilder ModelTransactionBuilder::addAssetQuantity(
        const interface::types::AccountIdType &account_id,
        const interface::types::AssetIdType &asset_id,
        const std::string &amount) {
      return ModelTransactionBuilder(
          builder_.addAssetQuantity(account_id, asset_id, amount));
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
        const std::vector<interface::types::PermissionNameType> &permissions) {
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
        const interface::types::PermissionNameType &permission) {
      return ModelTransactionBuilder(
          builder_.grantPermission(account_id, permission));
    }

    ModelTransactionBuilder ModelTransactionBuilder::revokePermission(
        const interface::types::AccountIdType &account_id,
        const interface::types::PermissionNameType &permission) {
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
        const interface::types::AccountIdType &account_id,
        const interface::types::AssetIdType &asset_id,
        const std::string &amount) {
      return ModelTransactionBuilder(
          builder_.subtractAssetQuantity(account_id, asset_id, amount));
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
