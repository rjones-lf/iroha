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

#include "bindings/model_builder.hpp"

namespace shared_model {
  namespace bindings {
    ModelBuilder ModelBuilder::creatorAccountId(
        const interface::types::AccountIdType &account_id) {
      return ModelBuilder(builder_.creatorAccountId(account_id));
    }

    ModelBuilder ModelBuilder::txCounter(uint64_t tx_counter) {
      return ModelBuilder(builder_.txCounter(tx_counter));
    }

    ModelBuilder ModelBuilder::createdTime(
        interface::types::TimestampType created_time) {
      return ModelBuilder(builder_.createdTime(created_time));
    }

    ModelBuilder ModelBuilder::assetQuantity(
        const interface::types::AccountIdType &account_id,
        const interface::types::AssetIdType &asset_id,
        const std::string &amount) {
      return ModelBuilder(builder_.addAssetQuantity(account_id, asset_id, amount));
    }

    ModelBuilder ModelBuilder::addPeer(
        const interface::types::AddressType &address,
        const interface::types::PubkeyType &peer_key) {
      return ModelBuilder(builder_.addPeer(address, peer_key));
    }

    ModelBuilder ModelBuilder::addSignatory(
        const interface::types::AddressType &account_id,
        const interface::types::PubkeyType &public_key) {
      return ModelBuilder(builder_.addSignatory(account_id, public_key));
    }

    ModelBuilder ModelBuilder::removeSignatory(
        const interface::types::AddressType &account_id,
        const interface::types::PubkeyType &public_key) {
      return ModelBuilder(builder_.removeSignatory(account_id, public_key));
    }

    ModelBuilder ModelBuilder::createAccount(
        const std::string &account_name,
        const interface::types::AddressType &domain_id,
        const interface::types::PubkeyType &main_pubkey) {
      return ModelBuilder(
          builder_.createAccount(account_name, domain_id, main_pubkey));
    }

    ModelBuilder ModelBuilder::createDomain(
        const interface::types::AddressType &domain_id,
        const interface::types::RoleIdType &default_role) {
      return ModelBuilder(builder_.createDomain(domain_id, default_role));
    }

    ModelBuilder ModelBuilder::setAccountQuorum(
        const interface::types::AddressType &account_id, uint32_t quorum) {
      return ModelBuilder(builder_.setAccountQuorum(account_id, quorum));
    }

    ModelBuilder ModelBuilder::transferAsset(
        const interface::types::AccountIdType &src_account_id,
        const interface::types::AccountIdType &dest_account_id,
        const interface::types::AssetIdType &asset_id,
        const std::string &description,
        const std::string &amount) {
      return ModelBuilder(builder_.transferAsset(
          src_account_id, dest_account_id, asset_id, description, amount));
    }

    proto::UnsignedWrapper<proto::Transaction> ModelBuilder::build() {
      return builder_.build();
    }
  }  // namespace bindings
}  // namespace shared_model
