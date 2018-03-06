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

#include <unordered_map>
#include "endpoint.grpc.pb.h"
#include "endpoint.pb.h"
#include "responses.pb.h"

#include "backend/protobuf/queries/proto_query.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "cache/cache.hpp"
#include "torii/processor/query_processor.hpp"

#include "logger/logger.hpp"

namespace torii {
  /**
   * Actual implementation of async QueryService.
   * ToriiServiceHandler::(SomeMethod)Handler calls a corresponding method in
   * this class.
   */
  class QueryService : public iroha::protocol::QueryService::Service {
   public:
    QueryService(std::shared_ptr<iroha::torii::QueryProcessor> query_processor);

    QueryService(const QueryService &) = delete;
    QueryService &operator=(const QueryService &) = delete;

    /**
     * actual implementation of async Find in QueryService
     * @param request - Query
     * @param response - QueryResponse
     */
    void Find(iroha::protocol::Query const &request,
              iroha::protocol::QueryResponse &response);

    grpc::Status Find(grpc::ServerContext *context,
                      const iroha::protocol::Query *request,
                      iroha::protocol::QueryResponse *response) override;

   private:
    std::shared_ptr<iroha::torii::QueryProcessor> query_processor_;

    iroha::cache::Cache<shared_model::crypto::Hash,
                        iroha::protocol::QueryResponse,
                        shared_model::crypto::Hash::Hasher>
        cache_;

    logger::Logger log_;
  };

}  // namespace torii

#endif  // TORII_QUERY_SERVICE_HPP
