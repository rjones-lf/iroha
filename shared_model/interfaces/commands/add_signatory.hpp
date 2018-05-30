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

#ifndef IROHA_SHARED_MODEL_ADD_SIGNATORY_HPP
#define IROHA_SHARED_MODEL_ADD_SIGNATORY_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Add new signatory to account
     */
    class AddSignatory : public ModelPrimitive<AddSignatory> {
     public:
      /**
       * @return New signatory is identified with public key
       */
      virtual const types::PubkeyType &pubkey() const = 0;
      /**
       * @return Account to which add new signatory
       */
      virtual const types::AccountIdType &accountId() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("AddSignatory")
            .append("pubkey", pubkey().toString())
            .append("account_id", accountId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return pubkey() == rhs.pubkey() and accountId() == rhs.accountId();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_ADD_SIGNATORY_HPP
