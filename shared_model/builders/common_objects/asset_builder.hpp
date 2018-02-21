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

#ifndef IROHA_ASSET_BUILDER_HPP
#define IROHA_ASSET_BUILDER_HPP

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/asset.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace builder {
    template <typename BuilderImpl, typename Validator>
    class AssetBuilder {
     public:
      BuilderResult<shared_model::interface::Asset> build() {
        auto asset = builder_.build();
        shared_model::validation::ReasonsGroupType reasons(
            "Asset Builder", shared_model::validation::GroupedReasons());
        shared_model::validation::Answer answer;
        validator_.validateAssetId(reasons, asset.assetId());
        validator_.validateDomainId(reasons, asset.domainId());
        validator_.validatePrecision(reasons, asset.precision());

        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }
        std::shared_ptr<shared_model::interface::Asset> asset_ptr(
            asset.copy());
        return iroha::expected::makeValue(
            shared_model::detail::PolymorphicWrapper<
                shared_model::interface::Asset>(asset_ptr));
      }

      AssetBuilder &assetId(
          const interface::types::AccountIdType &asset_id) {
        builder_ = builder_.assetId(asset_id);
        return *this;
      }

      AssetBuilder &domainId(
          const interface::types::DomainIdType &domain_id) {
        builder_ = builder_.domainId(domain_id);
        return *this;
      }

      AssetBuilder &precision(const interface::types::PrecisionType &precision) {
        builder_ = builder_.precision(precision);
        return *this;
      }

     private:
      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_ASSET_BUILDER_HPP
