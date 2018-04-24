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
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "validators/default_validator.hpp"

namespace torii {

  QueryService::QueryService(
      std::shared_ptr<iroha::torii::QueryProcessor> query_processor)
      : query_processor_(query_processor) {
    //    Subscribe on result from iroha
    query_processor_->queryNotifier().subscribe(
        [this](const std::shared_ptr<shared_model::interface::QueryResponse>
                   &iroha_response) {
          auto proto_response =
              static_cast<shared_model::proto::QueryResponse &>(*iroha_response)
                  .getTransport();

          auto hash = iroha_response->queryHash();
          cache_.addItem(hash, proto_response);
        });
  }

  void QueryService::Find(iroha::protocol::Query const &request,
                          iroha::protocol::QueryResponse &response) {
    shared_model::crypto::Hash hash;
    auto blobPayload = shared_model::proto::makeBlob(request.payload());
    hash = shared_model::crypto::DefaultHashProvider::makeHash(blobPayload);

    if (cache_.findItem(hash)) {
      // Query was already processed
      response.mutable_error_response()->set_reason(
          iroha::protocol::ErrorResponse::STATELESS_INVALID);
      return;
    }

    shared_model::proto::TransportBuilder<
        shared_model::proto::Query,
        shared_model::validation::DefaultQueryValidator>()
        .build(request)
        .match(
            [this, &hash, &response](
                const iroha::expected::Value<shared_model::proto::Query>
                    &query) {
              // Send query to iroha
              query_processor_->queryHandle(
                  std::make_shared<shared_model::proto::Query>(query.value));
              auto result_response = cache_.findItem(hash);
              if (result_response) {
                response.CopyFrom(result_response.value());
              } else {
                response.mutable_error_response()->set_reason(
                    iroha::protocol::ErrorResponse::NOT_SUPPORTED);
                cache_.addItem(hash, response);
              }
            },
            [&hash,
             &response](const iroha::expected::Error<std::string> &error) {
              response.set_query_hash(
                  shared_model::crypto::toBinaryString(hash));
              response.mutable_error_response()->set_reason(
                  iroha::protocol::ErrorResponse::STATELESS_INVALID);
              response.mutable_error_response()->set_message(std::move(error.error));
            });
  }

  grpc::Status QueryService::Find(grpc::ServerContext *context,
                                  const iroha::protocol::Query *request,
                                  iroha::protocol::QueryResponse *response) {
    Find(*request, *response);
    return grpc::Status::OK;
  }

}  // namespace torii
