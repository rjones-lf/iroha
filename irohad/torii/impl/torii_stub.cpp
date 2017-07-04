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

#include <torii/torii_stub.hpp>
#include <iostream>

namespace iroha {
  namespace torii {
    ToriiStub::ToriiStub(TransactionProcessor &transaction_processor,
                         QueryProcessor &query_processor) :
        transaction_processor_(transaction_processor),
        query_processor_(query_processor) {
      query_processor_.query_notifier()
          .subscribe([](auto q_response) {
            std::cout << "[\033[31mTorii\033[0m] response received" << std::endl;
          });
      transaction_processor_.transaction_notifier()
          .subscribe([](auto t_response) {
            std::cout << "[\033[31mTorii\033[0m] response received, msg: " << t_response.msg
                      << std::endl;
          });
    }

    void ToriiStub::get_query(dao::Client client, dao::Query &query) {
      std::cout << "[\033[31mTorii\033[0m] query received" << std::endl;
      query_processor_.query_handle(client, query);
    }

    void ToriiStub::get_transaction(dao::Client client, dao::Transaction &tx) {
      std::cout << "[\033[31mTorii\033[0m] transaction received" << std::endl;
      transaction_processor_.transaction_handle(client, tx);
    }
  } // namespace torii
} // namespace iroha