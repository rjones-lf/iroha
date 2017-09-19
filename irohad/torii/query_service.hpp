/*
Copyright 2017 Soramitsu Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef TORII_QUERY_SERVICE_HPP
#define TORII_QUERY_SERVICE_HPP

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>
#include <responses.pb.h>
#include <unordered_map>
#include "model/converters/pb_query_factory.hpp"
#include "model/converters/pb_query_response_factory.hpp"
#include "torii/processor/query_processor.hpp"

#include "logger/logger.hpp"

namespace torii {
  /**
   * Actual implementation of async QueryService.
   * ToriiServiceHandler::(SomeMethod)Handler calls a corresponding method in
   * this class.
   */
  class QueryService {
   public:
    QueryService(
        std::shared_ptr<iroha::model::converters::PbQueryFactory>
            pb_query_factory,
        std::shared_ptr<iroha::model::converters::PbQueryResponseFactory>
            pb_query_response_factory,
        std::shared_ptr<iroha::torii::QueryProcessor> query_processor);

    QueryService(const QueryService &) = delete;
    QueryService &operator=(const QueryService &) = delete;

    /**
     * actual implementation of async Find in QueryService
     * @param request - Query
     * @param response - QueryResponse
     */
    void FindAsync(iroha::protocol::Query const &request,
                   iroha::protocol::QueryResponse &response);

   private:
    std::shared_ptr<iroha::model::converters::PbQueryFactory> pb_query_factory_;
    std::shared_ptr<iroha::model::converters::PbQueryResponseFactory>
        pb_query_response_factory_;
    std::shared_ptr<iroha::torii::QueryProcessor> query_processor_;

    std::unordered_map<std::string, iroha::protocol::QueryResponse&>
        handler_map_;

    std::unordered_map<std::string, iroha::protocol::QueryResponse>
        old_queries_;


    logger::Logger log_;

  };

}  // namespace torii

#endif  // TORII_QUERY_SERVICE_HPP
