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

#ifndef IROHA_QUERY_PROCESSOR_IMPL_HPP
#define IROHA_QUERY_PROCESSOR_IMPL_HPP

#include "ametsuchi/storage.hpp"
#include "model/query_execution.hpp"
#include "torii/processor/query_processor.hpp"

namespace iroha {
  namespace torii {

    /**
     * QueryProcessor provides start point for queries in the whole system
     */
    class QueryProcessorImpl : public QueryProcessor {
     public:
      explicit QueryProcessorImpl(std::shared_ptr<ametsuchi::Storage> storage);

      /**
       * Register client query
       * @param query - client intent
       */
      void queryHandle(
          std::shared_ptr<shared_model::interface::Query> qry) override;

      /**
       * Subscribe for query responses
       * @return observable with query responses
       */
      rxcpp::observable<std::shared_ptr<shared_model::interface::QueryResponse>>
      queryNotifier() override;

     private:
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::QueryResponse>>
          subject_;
      std::shared_ptr<ametsuchi::Storage> storage_;
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_QUERY_PROCESSOR_IMPL_HPP
