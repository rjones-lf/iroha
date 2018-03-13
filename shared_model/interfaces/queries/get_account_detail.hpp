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

#ifndef IROHA_SHARED_MODEL_GET_ACCOUNT_DETAIL_HPP
#define IROHA_SHARED_MODEL_GET_ACCOUNT_DETAIL_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/get_account_detail.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Query for get all account's assets and balance
     */
    class GetAccountDetail : public PRIMITIVE(GetAccountDetail) {
     public:
      /**
       * @return account identifier
       */
      virtual const types::AccountIdType &accountId() const = 0;

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto oldModel = new iroha::model::GetAccountDetail;
        oldModel->account_id = accountId();
        return oldModel;
      }

#endif

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetAccountDetail")
            .append("account_id", accountId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_GET_ACCOUNT_DETAIL_HPP
