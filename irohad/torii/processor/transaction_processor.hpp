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

#ifndef IROHA_TRANSACTION_PROCESSOR_HPP
#define IROHA_TRANSACTION_PROCESSOR_HPP

#include <rxcpp/rx.hpp>

namespace shared_model {
  namespace interface {
    class Transaction;
    class TransactionResponse;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace torii {

    /**
     * Transaction processor is interface with start point
     * for processing transaction in the system
     */
    class TransactionProcessor {
     public:
      /**
       * Add transaction to the system for processing
       * @param transaction - transaction for processing
       */
      virtual void transactionHandle(
          std::shared_ptr<shared_model::interface::Transaction>
              transaction) = 0;

      /**
       * Subscribers will be notified with transaction status
       * @return observable for subscribing
       */
      virtual rxcpp::observable<
          std::shared_ptr<shared_model::interface::TransactionResponse>>
      transactionNotifier() = 0;

      virtual ~TransactionProcessor() = default;
    };
  }  // namespace torii
}  // namespace iroha
#endif  // IROHA_TRANSACTION_PROCESSOR_HPP
