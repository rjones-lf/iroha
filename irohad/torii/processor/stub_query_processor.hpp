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

#ifndef IROHA_STUB_QUERY_PROCESSOR_HPP
#define IROHA_STUB_QUERY_PROCESSOR_HPP

#include <torii/processor/query_processor.hpp>
#include <handler_map/handler_map.hpp>
#include <ametsuchi/block_query.hpp>
#include <ametsuchi/wsv_query.hpp>

namespace iroha {
  namespace torii {

    /**
     * QueryProcessor provides start point for queries in the whole system
     */
    class QueryProcessorStub : public QueryProcessor {
     public:

      explicit QueryProcessorStub(ametsuchi::WsvQuery &wsv,
                                  ametsuchi::BlockQuery &block);

      /**
       * Register client query
       * @param client - query emitter
       * @param query - client intent
       */
      void query_handle(dao::Client client, const dao::Query &query) override;

      /**
       * Subscribe for query responses
       * @return observable with query responses
       */
      rxcpp::observable<std::shared_ptr<dao::QueryResponse>> query_notifier() override;

     private:
      HandlerMap<dao::Query, void> handler_;
      rxcpp::subjects::subject<std::shared_ptr<dao::QueryResponse>> subject_;
      ametsuchi::WsvQuery &wsv_;
      ametsuchi::BlockQuery &block_;

      void handle_get_blocks(const dao::GetBlocks &blocks);

    };
  } //namespace torii
} //namespace iroha
#endif //IROHA_STUB_QUERY_PROCESSOR_HPP
