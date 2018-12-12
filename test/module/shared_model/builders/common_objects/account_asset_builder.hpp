/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ACCOUNT_ASSET_BUILDER_HPP
#define IROHA_ACCOUNT_ASSET_BUILDER_HPP

#include "interfaces/common_objects/account_asset.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

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
    class DEPRECATED AccountAssetBuilder
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
