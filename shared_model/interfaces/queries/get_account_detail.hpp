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

#include <boost/optional.hpp>

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Query for get all account's details; the algorithm of retrieving them is
     * the following:
     *  - if query has only accountId, all details about the specified account
     *    will be returned
     *  - if there is a key in a query, details written by all writers under
     *    this key will be returned
     *  - if there is a writer in a query, all details written by this writer
     *    will be returned
     *  - if there are both key and writer in a query, details written by this
     *    writer AND under this key will be returned
     */
    class GetAccountDetail : public ModelPrimitive<GetAccountDetail> {
     public:
      /**
       * @return account identifier
       */
      virtual const types::AccountIdType &accountId() const = 0;

      /**
       * @return key from key-value storage
       */
      virtual boost::optional<types::AccountDetailKeyType> key() const = 0;

      /**
       * @return account identifier of writer
       */
      virtual boost::optional<types::AccountIdType> writer() const = 0;

      std::string toString() const override;

      bool operator==(const ModelType &rhs) const override;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_GET_ACCOUNT_DETAIL_HPP
