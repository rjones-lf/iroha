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

#ifndef IROHA_SHARED_MODEL_GET_ACCOUNT_HPP
#define IROHA_SHARED_MODEL_GET_ACCOUNT_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "model/queries/get_account.hpp"

namespace shared_model {
  namespace interface {
    class GetAccount : public Primitive<GetAccount, iroha::model::GetAccount> {
     public:
      /**
       * @return Identity of user, for fetching data
       */
      virtual const types::AccountIdType &accountId() const = 0;

      OldModelType *makeOldModel() const override {
          auto oldModel = new iroha::model::GetAccount;
          oldModel->account_id = accountId();
          return oldModel;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetAccount")
            .append("account_id", accountId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_GET_ACCOUNT_HPP
