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

#ifndef IROHA_SHARED_MODEL_ACCOUNT_HPP
#define IROHA_SHARED_MODEL_ACCOUNT_HPP

#include "cryptography/hash.hpp"
#include "interfaces/base/hashable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "model/account.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * User identity information in the system
     */
    class Account : public Hashable<Account, iroha::model::Account> {
     public:
      /**
       * @return Identity of user, for fetching data
       */
      virtual const types::AccountIdType &accountId() const = 0;

      /**
       * @return Identity of domain, for fetching data
       */
      virtual const types::DomainIdType &domainId() const = 0;

      /**
       * @return Minimum quorum of signatures needed for transactions
       */
      virtual const types::QuorumType &quorum() const = 0;

      /**
       * Stringify the data.
       * @return the content of account asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Account")
            .append("accountId", accountId())
            .append("domainId", domainId())
            .append("quorum", std::to_string(quorum()))
            .finalize();
      }

      /**
       * Makes old model.
       * @return An allocated old model of account asset response.
       */
      OldModelType *makeOldModel() const override {
        OldModelType *oldModel = new OldModelType();
        oldModel->account_id = accountId();
        oldModel->domain_id = domainId();
        oldModel->quorum = quorum();
        return oldModel;
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_ACCOUNT_HPP
