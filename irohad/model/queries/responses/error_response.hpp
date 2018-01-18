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

#ifndef IROHA_ERROR_RESPONSE_HPP
#define IROHA_ERROR_RESPONSE_HPP

#include <string>
#include "model/query_response.hpp"

namespace iroha {
  namespace model {

    /**
     * Provide error answer with reason about error
     */
    struct ErrorResponse : public QueryResponse {
      /**
       * Reason of error
       */
      enum Reason {
        /**
         * signatures or created time are invalid
         */
        STATELESS_INVALID,
        /**
         * permissions are invalid
         */
        STATEFUL_INVALID,
        /**
         * when requested account does not exist
         */
        NO_ACCOUNT,
        /**
         * when requested asset does not exist
         */
        NO_ASSET,
        /**
         * No Roles found in the system
         */
        NO_ROLES,
        /**
         * when requested account asset does not exist
         */
        NO_ACCOUNT_ASSETS,
        /**
         * when requested account detail does not exist
         */
        NO_ACCOUNT_DETAIL,
        /**
         * when requested signatories does not exist
         */
        NO_SIGNATORIES,
        /**
         * when unidentified request was received
         */
        NOT_SUPPORTED
      };
      Reason reason{};
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_ERROR_RESPONSE_HPP
