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

#ifndef IROHA_TORII_STUB_HPP
#define IROHA_TORII_STUB_HPP

#include <torii/processor/query_processor.hpp>
#include <torii/processor/transaction_processor.hpp>
#include <model/model.hpp>

namespace iroha {
  namespace torii {

    /**
     * Stub for torii grpc service;
     */
    class ToriiStub {
     public:
      explicit ToriiStub(TransactionProcessor &transaction_processor,
                         QueryProcessor &query_processor);

      /**
       * Emulate query request from client
       * @param client - query owner
       * @param query - request for some storage information
       */
      void get_query(model::Client client, model::Query &query);

      /**
       * Emulate transaction request from client
       * @param client - transaction owner
       * @param tx - intent on changing state of ledger
       */
      void get_transaction(model::Client client, model::Transaction &tx);

     private:
      TransactionProcessor &transaction_processor_;
      QueryProcessor &query_processor_;
    };
  } // namespace torii
} // namespace iroha
#endif //IROHA_TORII_STUB_HPP
