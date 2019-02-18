/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/query_service.hpp"

#include "backend/protobuf/query_responses/proto_block_query_response.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "validators/default_validator.hpp"

namespace iroha {
  namespace torii {

    QueryService::QueryService(
        std::shared_ptr<iroha::torii::QueryProcessor> query_processor,
        std::shared_ptr<QueryFactoryType> query_factory,
        logger::Logger log)
        : query_processor_{std::move(query_processor)},
          query_factory_{std::move(query_factory)},
          log_{std::move(log)} {}

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

      query_factory_->build(request).match(
          [this, &hash, &response](
              const iroha::expected::Value<
                  std::unique_ptr<shared_model::interface::Query>> &query) {
            // Send query to iroha
            response = static_cast<shared_model::proto::QueryResponse &>(
                           *query_processor_->queryHandle(*query.value))
                           .getTransport();
            // TODO 18.02.2019 lebdron: IR-336 Replace cache
            // 0 is used as a dummy value
            cache_.addItem(hash, 0);
          },
          [&hash, &response](
              const iroha::expected::Error<QueryFactoryType::Error> &error) {
            response.set_query_hash(hash.hex());
            response.mutable_error_response()->set_reason(
                iroha::protocol::ErrorResponse::STATELESS_INVALID);
            response.mutable_error_response()->set_message(
                std::move(error.error.error));
          });
    }

    grpc::Status QueryService::Find(grpc::ServerContext *context,
                                    const iroha::protocol::Query *request,
                                    iroha::protocol::QueryResponse *response) {
      Find(*request, *response);
      return grpc::Status::OK;
    }

    grpc::Status QueryService::FetchCommits(
        grpc::ServerContext *context,
        const iroha::protocol::BlocksQuery *request,
        grpc::ServerWriter<iroha::protocol::BlockQueryResponse> *writer) {
      log_->debug("Fetching commits");
      shared_model::proto::TransportBuilder<
          shared_model::proto::BlocksQuery,
          shared_model::validation::DefaultSignedBlocksQueryValidator>()
          .build(*request)
          .match(
              [this, context, request, writer](
                  const iroha::expected::Value<shared_model::proto::BlocksQuery>
                      &query) {
                rxcpp::composite_subscription sub;
                query_processor_->blocksQueryHandle(query.value)
                    .as_blocking()
                    .subscribe(
                        sub,
                        [this, context, &sub, request, writer](
                            const std::shared_ptr<
                                shared_model::interface::BlockQueryResponse>
                                response) {
                          if (context->IsCancelled()) {
                            log_->debug("Unsubscribed");
                            sub.unsubscribe();
                          } else {
                            iroha::visit_in_place(
                                response->get(),
                                [this, writer, request](
                                    const shared_model::interface::BlockResponse
                                        &block_response) {
                                  log_->debug(
                                      "{} receives committed block",
                                      request->meta().creator_account_id());
                                  auto proto_block_response = static_cast<
                                      const shared_model::proto::BlockResponse
                                          &>(block_response);
                                  writer->Write(
                                      proto_block_response.getTransport());
                                },
                                [this, writer, request](
                                    const shared_model::interface::
                                        BlockErrorResponse
                                            &block_error_response) {
                                  log_->debug(
                                      "{} received error with message: {}",
                                      request->meta().creator_account_id(),
                                      block_error_response.message());
                                  auto proto_block_error_response =
                                      static_cast<const shared_model::proto::
                                                      BlockErrorResponse &>(
                                          block_error_response);
                                  writer->WriteLast(
                                      proto_block_error_response.getTransport(),
                                      grpc::WriteOptions());
                                });
                          }
                        });
              },
              [this, writer](const auto &error) {
                log_->debug("Stateless invalid: {}", error.error);
                iroha::protocol::BlockQueryResponse response;
                response.mutable_block_error_response()->set_message(
                    std::move(error.error));
                writer->WriteLast(response, grpc::WriteOptions());
              });

      return grpc::Status::OK;
    }

  }  // namespace torii
}  // namespace iroha
