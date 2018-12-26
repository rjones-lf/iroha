/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ASSET_BUILDER_HPP
#define IROHA_ASSET_BUILDER_HPP

#include "interfaces/common_objects/asset.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

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
    class DEPRECATED AssetBuilder
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
