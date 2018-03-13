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

#ifndef IROHA_SHARED_MODEL_SET_ACCOUNT_DETAIL_HPP
#define IROHA_SHARED_MODEL_SET_ACCOUNT_DETAIL_HPP

#ifndef DISABLE_BACKWARD
#include "model/commands/set_account_detail.hpp"
#endif

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Set key-value pair of given account
     */
    class SetAccountDetail : public PRIMITIVE(SetAccountDetail) {
     public:
      /**
       * @return Identity of user to set account detail
       */
      virtual const types::AccountIdType &accountId() const = 0;

      /**
       * @return key of data to store in the account
       */
      virtual const types::AccountDetailKeyType &key() const = 0;

      /**
       * @return detail value to store by given key
       */
      virtual const types::AccountDetailValueType &value() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("SetAccountDetail")
            .append("account_id", accountId())
            .append("key", key())
            .append("value", value())
            .finalize();
      }

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto oldModel = new OldModelType;
        oldModel->account_id = accountId();
        oldModel->key = key();
        oldModel->value = value();
        return oldModel;
      }

#endif

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId() and key() == rhs.key()
            and value() == rhs.value();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_SET_ACCOUNT_DETAIL_HPP
