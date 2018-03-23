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

#ifndef IROHA_SHARED_MODEL_ACCOUNT_DETAIL_RESPONSE_HPP
#define IROHA_SHARED_MODEL_ACCOUNT_DETAIL_RESPONSE_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/account_asset.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"
#include "utils/visitor_apply_for_all.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/responses/account_detail_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Provide response with account asset
     */
    class AccountDetailResponse : public PRIMITIVE(AccountDetailResponse) {
     public:
      /// Detail
      using DetailType = std::string;
      /**
       * @return Account has Asset model
       */
      virtual const DetailType &detail() const = 0;

      /**
       * Stringify the data.
       * @return string representation of data.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("AccountDetailResponse")
            .append(detail())
            .finalize();
      }

      /**
       * Implementation of operator ==
       * @param rhs - the right-hand side of AccountAssetResponse object
       * @return true if they are same.
       */
      bool operator==(const ModelType &rhs) const override {
        return detail() == rhs.detail();
      }

#ifndef DISABLE_BACKWARD
      /**
       * Makes old model.
       * @return An allocated old model of account asset response.
       */
      OldModelType *makeOldModel() const override {
        OldModelType *oldModel = new OldModelType();
        oldModel->detail = detail();
        return oldModel;
      }

#endif
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_ACCOUNT_DETAIL_RESPONSE_HPP
