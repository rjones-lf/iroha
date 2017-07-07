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

#ifndef IROHA_QUERY_PROCESSOR_HPP
#define IROHA_QUERY_PROCESSOR_HPP

#include <model/model.hpp>
#include <rxcpp/rx.hpp>

namespace iroha {
  namespace torii {

    /**
     * QueryProcessor provides start point for queries in the whole system
     */
    class QueryProcessor {
     public:

      /**
       * Register client query
       * @param client - query emitter
       * @param query - client intent
       */
      virtual void query_handle(model::Client client,
                                const model::Query &query) = 0;

      /**
       * Subscribe for query responses
       * @return observable with query responses
       */
      virtual rxcpp::observable<std::shared_ptr<model::QueryResponse>> query_notifier() = 0;

      virtual ~QueryProcessor() {
      };
    };
  } //namespace torii
} //namespace iroha

#endif //IROHA_QUERY_PROCESSOR_HPP
