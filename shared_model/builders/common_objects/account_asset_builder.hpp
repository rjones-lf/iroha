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

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {
    /**
     * AccountAssetBuilder is a class, used for construction of AccountAsset
     * objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class AccountAssetBuilder
        : public CommonObjectBuilder<interface::AccountAsset,
                                     BuilderImpl,
                                     Validator> {
     public:
      AccountAssetBuilder accountId(
          const interface::types::AccountIdType &account_id) {
        AccountAssetBuilder copy(*this);
        copy.builder_ = this->builder_.accountId(account_id);
        return copy;
      }

      AccountAssetBuilder assetId(
          const interface::types::AssetIdType &asset_id) {
        AccountAssetBuilder copy(*this);
        copy.builder_ = this->builder_.assetId(asset_id);
        return copy;
      }

      AccountAssetBuilder balance(const interface::Amount &amount) {
        AccountAssetBuilder copy(*this);
        copy.builder_ = this->builder_.balance(amount);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Account Asset Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::AccountAsset &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validateAccountId(reasons, object.accountId());
        this->validator_.validateAssetId(reasons, object.assetId());
        // Do not validate balance, since its amount can be 0, which is
        // forbidden by validation

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model
#endif  // IROHA_ACCOUNT_ASSET_BUILDER_HPP
