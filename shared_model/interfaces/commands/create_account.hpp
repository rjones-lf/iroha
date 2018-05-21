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

#ifndef IROHA_SHARED_MODEL_CREATE_ACCOUNT_HPP
#define IROHA_SHARED_MODEL_CREATE_ACCOUNT_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Create acccount in Iroha domain
     */
    class CreateAccount : public ModelPrimitive<CreateAccount> {
     public:
      /**
       * @return Name of the account to create in Iroha
       */
      virtual const types::AccountNameType &accountName() const = 0;
      /**
       * @return Iroha domain in which account will be created
       */
      virtual const types::DomainIdType &domainId() const = 0;
      /**
       * @return Initial account public key
       */
      virtual const types::PubkeyType &pubkey() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("CreateAccount")
            .append("account_name", accountName())
            .append("domain_id", domainId())
            .append(pubkey().toString())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountName() == rhs.accountName()
            and domainId() == rhs.domainId() and pubkey() == rhs.pubkey();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_CREATE_ACCOUNT_HPP
