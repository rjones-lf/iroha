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

#ifndef IROHA_SHARED_MODEL_ASSET_RESPONSE_HPP
#define IROHA_SHARED_MODEL_ASSET_RESPONSE_HPP

#include <new>
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/asset.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"
#include "utils/visitor_apply_for_all.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/responses/asset_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Provide response with asset
     */
    class AssetResponse : public PRIMITIVE(AssetResponse) {
     public:
      /**
       * @return Attached asset
       */
      virtual const Asset &asset() const = 0;

      /**
       * Stringify the data.
       * @return string representation of data.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("AssetResponse")
            .append(asset().toString())
            .finalize();
      }

      /**
       * @return true if the data are same.
       */
      bool operator==(const ModelType &rhs) const override {
        return asset() == rhs.asset();
      }

#ifndef DISABLE_BACKWARD
      /**
       * Makes old model.
       * @return An allocated old model of asset response.
       */
      OldModelType *makeOldModel() const override {
        OldModelType *oldModel = new OldModelType();
        using OldAssetType = decltype(oldModel->asset);
        /// Use shared_ptr and placement-new to copy new model field to
        /// oldModel's field and to return raw pointer
        auto p = std::shared_ptr<OldAssetType>(asset().makeOldModel());
        new (&oldModel->asset) OldAssetType(*p);
        return oldModel;
      }

#endif
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_ASSET_RESPONSE_HPP
