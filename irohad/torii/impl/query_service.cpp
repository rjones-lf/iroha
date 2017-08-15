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

#include "torii/query_service.hpp"

namespace torii {

  QueryService::QueryService(
      std::shared_ptr<iroha::model::converters::PbQueryFactory>
          pb_query_factory,
      std::shared_ptr<iroha::model::converters::PbQueryResponseFactory>
          pb_query_response_factory,
      std::shared_ptr<iroha::torii::QueryProcessor> query_processor)
      : pb_query_factory_(pb_query_factory),
        pb_query_response_factory_(pb_query_response_factory),
        query_processor_(query_processor) {
    log_ = logger::log("QueryService");
    // Subscribe on result from iroha
    query_processor_->queryNotifier().subscribe([this](auto iroha_response) {
      // Find client to respond
      auto res = handler_map_.find(iroha_response->query_hash.to_string());
      // Serialize to proto an return to response
      res->second =
          pb_query_response_factory_->serialize(iroha_response).value();

    });
  }

  void QueryService::FindAsync(iroha::protocol::Query const& request,
                               iroha::protocol::QueryResponse& response) {
    // Get iroha model query
    auto query = pb_query_factory_->deserialize(request);

    if (not query.has_value()) {
      response.mutable_error_response()->set_reason(
          iroha::protocol::ErrorResponse::NOT_SUPPORTED);
      return;
    }

    auto hash = query.value()->query_hash.to_string();

    if (handler_map_.count(hash) > 0) {
      // Query was already processed
      response.mutable_error_response()->set_reason(
          iroha::protocol::ErrorResponse::STATELESS_INVALID);
      return;
    }

    // Query - response relationship
    handler_map_.emplace(hash, response);
    // Send query to iroha
    query_processor_->queryHandle(query.value());
  }
}  // namespace torii
