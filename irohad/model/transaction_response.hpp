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

#ifndef IROHA_TRANSACTION_RESPONSE_HPP
#define IROHA_TRANSACTION_RESPONSE_HPP

#include <model/client.hpp>
#include <model/transaction.hpp>

namespace iroha {
  namespace model {

    /**
     * Transaction response is data with status during transaction lifecycle
     */
    struct TransactionResponse {
      /**
       * Processed transaction
       */
      std::string tx_hash{};

      enum Status {
        STATELESS_VALIDATION_FAILED,   // stateless validation failed
        STATELESS_VALIDATION_SUCCESS,  // stateless validation passed
        STATEFUL_VALIDATION_FAILED,    // stateful validation failed
        STATEFUL_VALIDATION_SUCCESS,   // stateful validation passed
        COMMITTED,                     // tx pipeline succeeded, tx is committed
        NOT_RECEIVED  // transaction is not in handler map
      };

      Status current_status{};

      virtual ~TransactionResponse() = default;
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_TRANSACTION_RESPONSE_HPP
