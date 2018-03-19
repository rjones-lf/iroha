/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include <thread>

#include "ametsuchi/block_query.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "common/types.hpp"
#include "endpoint.pb.h"
#include "interfaces/base/hashable.hpp"
#include "torii/command_service.hpp"
#include "validators/default_validator.hpp"
#include "validators/transaction_validator.hpp"

using namespace std::chrono_literals;

namespace torii {

  CommandService::CommandService(
      std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
      std::shared_ptr<iroha::ametsuchi::BlockQuery> block_query,
      std::chrono::milliseconds proposal_delay)
      : tx_processor_(tx_processor),
        block_query_(block_query),
        proposal_delay_(proposal_delay),
        start_tx_processing_duration_(1s),
        cache_(std::make_shared<CacheType>()) {
    // Notifier for all clients
    tx_processor_->transactionNotifier().subscribe(
        [this](
            std::shared_ptr<iroha::model::TransactionResponse> iroha_response) {
          // Find response in cache
          shared_model::crypto::Hash tx_hash(iroha_response->tx_hash);
          auto res = cache_->findItem(tx_hash);
          if (not res) {
            iroha::protocol::ToriiResponse response;
            response.set_tx_hash(shared_model::crypto::toBinaryString(tx_hash));
            response.set_tx_status(iroha::protocol::NOT_RECEIVED);
            cache_->addItem(tx_hash, response);
            return;
          }

          auto proto_status =
              convertStatusToProto(iroha_response->current_status);
          res->set_tx_status(proto_status);
          cache_->addItem(tx_hash, *res);
        });
  }

  void CommandService::Torii(const iroha::protocol::Transaction &request) {
    shared_model::crypto::Hash tx_hash;
    iroha::protocol::ToriiResponse response;

    shared_model::proto::TransportBuilder<
        shared_model::proto::Transaction,
        shared_model::validation::DefaultSignableTransactionValidator>()
        .build(request)
        .match(
            [this, &tx_hash, &response](
                // success case
                const iroha::expected::Value<shared_model::proto::Transaction>
                    &iroha_tx) {
              tx_hash = iroha_tx.value.hash();
              if (cache_->findItem(tx_hash)) {
                return;
              }

              // setting response
              response.set_tx_hash(tx_hash.toString());
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);

              cache_->addItem(tx_hash, response);
              // Send transaction to iroha
              tx_processor_->transactionHandle(
                  std::shared_ptr<iroha::model::Transaction>(
                      iroha_tx.value.makeOldModel()));
            },
            [this, &tx_hash, &request, &response](const auto &error) {
              // getting hash from invalid transaction
              auto blobPayload =
                  shared_model::proto::makeBlob(request.payload());
              tx_hash =
                  shared_model::proto::Transaction::HashProviderType::makeHash(
                      blobPayload);

              // setting response
              response.set_tx_hash(
                  shared_model::crypto::toBinaryString(tx_hash));
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
            });

    cache_->addItem(tx_hash, response);
  }

  grpc::Status CommandService::Torii(
      grpc::ServerContext *context,
      const iroha::protocol::Transaction *request,
      google::protobuf::Empty *response) {
    Torii(*request);
    return grpc::Status::OK;
  }

  void CommandService::Status(const iroha::protocol::TxStatusRequest &request,
                              iroha::protocol::ToriiResponse &response) {
    auto tx_hash = shared_model::crypto::Hash(request.tx_hash());
    auto resp = cache_->findItem(tx_hash);
    if (resp) {
      response.CopyFrom(*resp);
    } else {
      response.set_tx_hash(request.tx_hash());
      if (block_query_->getTxByHashSync(
              shared_model::crypto::Hash(request.tx_hash()))) {
        response.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
      } else {
        response.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      }
      cache_->addItem(tx_hash, response);
    }
  }

  grpc::Status CommandService::Status(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      iroha::protocol::ToriiResponse *response) {
    Status(*request, *response);
    return grpc::Status::OK;
  }

  void CommandService::StatusStream(
      iroha::protocol::TxStatusRequest const &request,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> &response_writer) {
    auto resp = cache_->findItem(shared_model::crypto::Hash(request.tx_hash()));
    checkCacheAndSend(resp, response_writer);

    bool finished = false;
    auto subscription = rxcpp::composite_subscription();
    auto request_hash = request.tx_hash();

    /// condition variable to ensure that current method will not return before
    /// transaction is processed or a timeout reached. It blocks current thread
    /// and waits for thread from subscribe() to unblock.
    std::condition_variable cv;

    tx_processor_->transactionNotifier()
        .filter([&request_hash](auto response) {
          return response->tx_hash == request_hash;
        })
        .subscribe(subscription,
                   [&](std::shared_ptr<iroha::model::TransactionResponse>
                           iroha_response) {
                     iroha::protocol::ToriiResponse resp_sub;
                     resp_sub.set_tx_hash(request_hash);
                     auto proto_status =
                         convertStatusToProto(iroha_response->current_status);
                     resp_sub.set_tx_status(proto_status);

                     if (isFinalStatus(proto_status)) {
                       response_writer.WriteLast(resp_sub,
                                                 grpc::WriteOptions());
                       subscription.unsubscribe();
                       finished = true;
                       cv.notify_one();
                     } else {
                       response_writer.Write(resp_sub);
                     }
                   });

    std::mutex wait_subscription;
    std::unique_lock<std::mutex> lock(wait_subscription);
    /// we expect that start_tx_processing_duration_ will be enough
    /// to at least start tx processing.
    /// Otherwise we think there is no such tx at all.
    cv.wait_for(lock, start_tx_processing_duration_);
    if (not finished) {
      if (not resp) {
        subscription.unsubscribe();
        iroha::protocol::ToriiResponse resp_none;
        resp_none.set_tx_hash(request_hash);
        resp_none.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
        response_writer.WriteLast(resp_none, grpc::WriteOptions());
      } else {
        /// Tx processing was started but still unfinished. We give it
        /// 2*proposal_delay time until timeout.
        cv.wait_for(lock, 2 * proposal_delay_);
      }
    }
  }

  grpc::Status CommandService::StatusStream(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> *response_writer) {
    StatusStream(*request, *response_writer);
    return grpc::Status::OK;
  }

  void CommandService::checkCacheAndSend(
      const boost::optional<iroha::protocol::ToriiResponse> &resp,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> &response_writer)
      const {
    if (resp) {
      if (isFinalStatus(resp->tx_status())) {
        response_writer.WriteLast(*resp, grpc::WriteOptions());
        return;
      }
      response_writer.Write(*resp);
    }
  }

  iroha::protocol::TxStatus CommandService::convertStatusToProto(
      const iroha::model::TransactionResponse::Status &status) {
    iroha::protocol::TxStatus proto_status;
    switch (status) {
      case iroha::model::TransactionResponse::STATELESS_VALIDATION_FAILED:
        proto_status = iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED;
        break;
      case iroha::model::TransactionResponse::STATELESS_VALIDATION_SUCCESS:
        proto_status = iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS;
        break;
      case iroha::model::TransactionResponse::STATEFUL_VALIDATION_FAILED:
        proto_status = iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED;
        break;
      case iroha::model::TransactionResponse::STATEFUL_VALIDATION_SUCCESS:
        proto_status = iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS;
        break;
      case iroha::model::TransactionResponse::IN_PROGRESS:
        proto_status = iroha::protocol::TxStatus::IN_PROGRESS;
        break;
      case iroha::model::TransactionResponse::COMMITTED:
        proto_status = iroha::protocol::TxStatus::COMMITTED;
        break;
      case iroha::model::TransactionResponse::NOT_RECEIVED:
      default:
        proto_status = iroha::protocol::TxStatus::NOT_RECEIVED;
        break;
    }
    return proto_status;
  }

  bool CommandService::isFinalStatus(
      const iroha::protocol::TxStatus &status) const {
    using namespace iroha::protocol;
    std::vector<TxStatus> final_statuses = {
        TxStatus::STATELESS_VALIDATION_FAILED,
        TxStatus::STATEFUL_VALIDATION_FAILED,
        TxStatus::NOT_RECEIVED,
        TxStatus::COMMITTED};
    return (std::find(
               std::begin(final_statuses), std::end(final_statuses), status))
        != std::end(final_statuses);
  }
}  // namespace torii
