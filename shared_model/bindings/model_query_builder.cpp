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

#include "bindings/model_query_builder.hpp"

namespace shared_model {
  namespace bindings {
    ModelQueryBuilder::ModelQueryBuilder() {
      *this = creatorAccountId("").createdTime(0).queryCounter(0);
    }

    ModelQueryBuilder ModelQueryBuilder::createdTime(
        interface::types::TimestampType created_time) {
      return ModelQueryBuilder(builder_.createdTime(created_time));
    }

    ModelQueryBuilder ModelQueryBuilder::creatorAccountId(
        const interface::types::AccountIdType &creator_account_id) {
      return ModelQueryBuilder(builder_.creatorAccountId(creator_account_id));
    }

    ModelQueryBuilder ModelQueryBuilder::queryCounter(
        interface::types::CounterType query_counter) {
      return ModelQueryBuilder(builder_.queryCounter(query_counter));
    }

    ModelQueryBuilder ModelQueryBuilder::getAccount(
        const interface::types::AccountIdType &account_id) {
      return ModelQueryBuilder(builder_.getAccount(account_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getSignatories(
        const interface::types::AccountIdType &account_id) {
      return ModelQueryBuilder(builder_.getSignatories(account_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getAccountTransactions(
        const interface::types::AccountIdType &account_id) {
      return ModelQueryBuilder(builder_.getAccountTransactions(account_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getAccountAssetTransactions(
        const interface::types::AccountIdType &account_id,
        const interface::types::AssetIdType &asset_id) {
      return ModelQueryBuilder(
          builder_.getAccountAssetTransactions(account_id, asset_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getAccountAssets(
        const interface::types::AccountIdType &account_id,
        const interface::types::AssetIdType &asset_id) {
      return ModelQueryBuilder(builder_.getAccountAssets(account_id, asset_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getRoles() {
      return ModelQueryBuilder(builder_.getRoles());
    }

    ModelQueryBuilder ModelQueryBuilder::getAssetInfo(
        const interface::types::AssetIdType &asset_id) {
      return ModelQueryBuilder(builder_.getAssetInfo(asset_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getRolePermissions(
        const interface::types::RoleIdType &role_id) {
      return ModelQueryBuilder(builder_.getRolePermissions(role_id));
    }

    ModelQueryBuilder ModelQueryBuilder::getTransactions(
        const std::vector<crypto::Hash> &hashes) {
      return ModelQueryBuilder(builder_.getTransactions(hashes));
    }

    ModelQueryBuilder ModelQueryBuilder::getAccountDetail(
        const interface::types::AccountIdType &account_id,
        const interface::types::DetailType &detail) {
      return ModelQueryBuilder(builder_.getAccountDetail(account_id, detail));
    }

    proto::UnsignedWrapper<proto::Query> ModelQueryBuilder::build() {
      return builder_.build();
    }
  }  // namespace bindings
}  // namespace shared_model
