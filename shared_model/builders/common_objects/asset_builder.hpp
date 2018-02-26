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

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {

    /**
     * AssetBuilder is a class, used for construction of Asset objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class AssetBuilder
        : public CommonObjectBuilder<interface::Asset, BuilderImpl, Validator> {
     public:
      AssetBuilder assetId(const interface::types::AccountIdType &asset_id) {
        AssetBuilder copy(*this);
        copy.builder_ = this->builder_.assetId(asset_id);
        return copy;
      }

      AssetBuilder domainId(const interface::types::DomainIdType &domain_id) {
        AssetBuilder copy(*this);
        copy.builder_ = this->builder_.domainId(domain_id);
        return copy;
      }

      AssetBuilder precision(const interface::types::PrecisionType &precision) {
        AssetBuilder copy(*this);
        copy.builder_ = this->builder_.precision(precision);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Asset Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Asset &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validateAssetId(reasons, object.assetId());
        this->validator_.validateDomainId(reasons, object.domainId());
        this->validator_.validatePrecision(reasons, object.precision());

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_ASSET_BUILDER_HPP
