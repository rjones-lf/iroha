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

#ifndef IROHA_ACCOUNT_ASSET_BUILDER_HPP
#define IROHA_ACCOUNT_ASSET_BUILDER_HPP

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/account_asset.hpp"

namespace shared_model {
  namespace builder {
    template <typename BuilderImpl, typename Validator>
    class AccountAssetBuilder {
     public:
      BuilderResult<shared_model::interface::AccountAsset> build() {
        auto account_asset = builder_.build();

        shared_model::validation::ReasonsGroupType reasons(
            "Account Asset Builder", shared_model::validation::GroupedReasons());
        shared_model::validation::Answer answer;
        validator_.validateAccountId(reasons, account_asset.accountId());
        validator_.validateAssetId(reasons, account_asset.assetId());
        // Do not validate balance, since its amount can be 0, which is forbidden by validation

        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }

        std::shared_ptr<shared_model::interface::AccountAsset> account_asset_ptr(
            account_asset.copy());
        return iroha::expected::makeValue(
            shared_model::detail::PolymorphicWrapper<
                shared_model::interface::AccountAsset>(account_asset_ptr));
      }

      AccountAssetBuilder &accountId(
          const interface::types::AccountIdType &account_id) {
        builder_ = builder_.accountId(account_id);
        return *this;
      }

      AccountAssetBuilder &assetId(
          const interface::types::AssetIdType &asset_id) {
        builder_ = builder_.assetId(asset_id);
        return *this;
      }

      AccountAssetBuilder &balance(
          const interface::Amount &amount) {
        builder_ = builder_.balance(amount);
        return *this;
      }

     private:
      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model
#endif  // IROHA_ACCOUNT_ASSET_BUILDER_HPP
