/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP
#define IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP

#include "backend/protobuf/common_objects/account_asset.hpp"
#include "responses.pb.h"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace proto {
    /**
     * AccountAssetBuilder is used to construct AccountAsset proto objects with initialized
     * protobuf implementation
     */
    class AccountAssetBuilder {
     public:
      shared_model::proto::AccountAsset build() {
        return shared_model::proto::AccountAsset(account_asset_);
      }

      AccountAssetBuilder &accountId(
          const interface::types::AccountIdType &account_id) {
        account_asset_.set_account_id(account_id);
        return *this;
      }

      AccountAssetBuilder &assetId(
          const interface::types::AssetIdType &asset_id) {
        account_asset_.set_asset_id(asset_id);
        return *this;
      }

      AccountAssetBuilder &balance(const interface::Amount &amount) {
        // TODO: 14.02.2018 nickaleks add proper amount initialization IR-972
        auto *amount_proto = new iroha::protocol::Amount();
        amount_proto->mutable_value()->set_first(
            amount.intValue().template convert_to<uint64_t>());
        amount_proto->set_precision(amount.precision());
        account_asset_.set_allocated_balance(amount_proto);
        return *this;
      }

     private:
      iroha::protocol::AccountAsset account_asset_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP
