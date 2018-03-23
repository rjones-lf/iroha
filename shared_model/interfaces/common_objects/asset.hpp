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

#ifndef IROHA_SHARED_MODEL_ASSET_HPP
#define IROHA_SHARED_MODEL_ASSET_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

#ifndef DISABLE_BACKWARD
#include "model/asset.hpp"
#endif

namespace shared_model {
  namespace interface {

    /**
     * Representation of valuable goods in the system
     */
    class Asset : public PRIMITIVE(Asset) {
     public:
      /**
       * @return Identity of asset
       */
      virtual const types::AccountIdType &assetId() const = 0;

      /**
       * @return Identity of domain
       */
      virtual const types::DomainIdType &domainId() const = 0;

      /**
       * @return Asset's fixed precision
       */
      virtual types::PrecisionType precision() const = 0;

      /**
       * Stringify the data.
       * @return the content of asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Asset")
            .append("assetId", assetId())
            .append("domainId", domainId())
            .append("precision", std::to_string(precision()))
            .finalize();
      }

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const ModelType &rhs) const override {
        return assetId() == rhs.assetId() and domainId() == rhs.domainId()
            and precision() == rhs.precision();
      }

#ifndef DISABLE_BACKWARD
      /**
       * Makes old model.
       * @return An allocated old model of account asset response.
       */
      OldModelType *makeOldModel() const override {
        OldModelType *oldModel = new OldModelType();
        oldModel->asset_id = assetId();
        oldModel->domain_id = domainId();
        oldModel->precision = precision();
        return oldModel;
      }
#endif
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_ASSET_HPP
