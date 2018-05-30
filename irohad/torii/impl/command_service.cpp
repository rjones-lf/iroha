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

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"

#include "ametsuchi/block_query.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "common/byteutils.hpp"
#include "common/types.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "endpoint.pb.h"
#include "torii/command_service.hpp"
#include "validators/default_validator.hpp"

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
        cache_(std::make_shared<CacheType>()),
        log_(logger::log("CommandService")) {
    // Notifier for all clients
    tx_processor_->transactionNotifier().subscribe([this](auto iroha_response) {
      // Find response in cache
      auto proto_response =
          std::static_pointer_cast<shared_model::proto::TransactionResponse>(
              iroha_response);
      auto tx_hash = proto_response->transactionHash();
      cache_->addItem(tx_hash, proto_response->getTransport());
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
                iroha::expected::Value<shared_model::proto::Transaction>
                    &iroha_tx) {
              tx_hash = iroha_tx.value.hash();
              if (cache_->findItem(tx_hash) and iroha_tx.value.quorum() < 2) {
                log_->warn("Found transaction {} in cache, ignoring",
                           tx_hash.hex());
                return;
              }

              // setting response
              response.set_tx_hash(tx_hash.toString());
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);

              // Send transaction to iroha
              tx_processor_->transactionHandle(
                  std::make_shared<shared_model::proto::Transaction>(
                      std::move(iroha_tx.value)));
            },
            [this, &tx_hash, &request, &response](const auto &error) {
              // getting hash from invalid transaction
              auto blobPayload =
                  shared_model::proto::makeBlob(request.payload());
              tx_hash = shared_model::crypto::DefaultHashProvider::makeHash(
                  blobPayload);
              log_->warn("Stateless invalid tx: {}, hash: {}",
                         error.error,
                         tx_hash.hex());

              // setting response
              response.set_tx_hash(
                  shared_model::crypto::toBinaryString(tx_hash));
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
              response.set_error_message(std::move(error.error));
            });
    log_->debug("Torii: adding item to cache: {}, status {} ",
                tx_hash.hex(),
                response.tx_status());
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
      if (block_query_->hasTxWithHash(
              shared_model::crypto::Hash(request.tx_hash()))) {
        response.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
      } else {
        log_->warn("Asked non-existing tx: {}",
                   iroha::bytestringToHexstring(request.tx_hash()));
        response.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      }
      log_->debug("Status: adding item to cache: {}, status {}",
                  tx_hash.hex(),
                  response.tx_status());
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
    if (checkCacheAndSend(resp, response_writer)) {
      return;
    }
    auto finished = std::make_shared<std::atomic<bool>>(false);
    auto subscription = rxcpp::composite_subscription();
    auto request_hash =
        std::make_shared<shared_model::crypto::Hash>(request.tx_hash());

    /// Condition variable to ensure that current method will not return before
    /// transaction is processed or a timeout reached. It blocks current thread
    /// and waits for thread from subscribe() to unblock.
    auto cv = std::make_shared<std::condition_variable>();

    log_->debug("StatusStream before subscribe(), hash: {}",
                request_hash->hex());

    tx_processor_->transactionNotifier()
        .filter([&request_hash](auto response) {
          return response->transactionHash() == *request_hash;
        })
        .subscribe(
            subscription,
            [&](std::shared_ptr<shared_model::interface::TransactionResponse>
                    iroha_response) {
              auto proto_response = std::static_pointer_cast<
                  shared_model::proto::TransactionResponse>(iroha_response);

              log_->debug("subscribe new status: {}, hash {}",
                          proto_response->toString(),
                          proto_response->transactionHash().hex());

              iroha::protocol::ToriiResponse resp_sub =
                  proto_response->getTransport();

              if (isFinalStatus(resp_sub.tx_status())) {
                response_writer.WriteLast(resp_sub, grpc::WriteOptions());
                *finished = true;
                cv->notify_one();
              } else {
                response_writer.Write(resp_sub);
              }
            });

    std::mutex wait_subscription;
    std::unique_lock<std::mutex> lock(wait_subscription);

    log_->debug("StatusStream waiting start, hash: {}", request_hash->hex());

    /// we expect that start_tx_processing_duration_ will be enough
    /// to at least start tx processing.
    /// Otherwise we think there is no such tx at all.
    cv->wait_for(lock, start_tx_processing_duration_);

    log_->debug("StatusStream waiting finish, hash: {}", request_hash->hex());

    if (not *finished) {
      if (not resp) {
        log_->warn("StatusStream request processing timeout, hash: {}",
                   request_hash->hex());
        // TODO 05/03/2018 andrei IR-1046 Server-side shared model object
        // factories with move semantics
        auto resp_none = shared_model::proto::TransactionStatusBuilder()
                             .txHash(*request_hash)
                             .notReceived()
                             .build();
        response_writer.WriteLast(resp_none.getTransport(),
                                  grpc::WriteOptions());
      } else {
        log_->debug(
            "Tx processing was started but unfinished, awaiting more, hash: {}",
            request_hash->hex());
        /// We give it 2*proposal_delay time until timeout.
        cv->wait_for(lock, 2 * proposal_delay_);

        /// status can be in the cache if it was finalized before we subscribed
        if (not *finished) {
          log_->debug("Transaction {} still not finished", request_hash->hex());

          auto cache_second_check =
              cache_->findItem(shared_model::crypto::Hash(request.tx_hash()));
          log_->debug("Status in cache: {}", cache_second_check->tx_status());

          /// final status means the case from a comment above
          /// if it's not - let's ignore it for now
          if (isFinalStatus(cache_second_check->tx_status())) {
            log_->warn("Transaction was finalized before subscription");
            response_writer.WriteLast(*resp, grpc::WriteOptions());
          }
        }
      }
    } else {
      log_->debug("StatusStream request processed successfully, hash: {}",
                  request_hash->hex());
    }
    subscription.unsubscribe();
    log_->debug("StatusStream unsubscribed");
  }

  grpc::Status CommandService::StatusStream(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> *response_writer) {
    StatusStream(*request, *response_writer);
    return grpc::Status::OK;
  }

  bool CommandService::checkCacheAndSend(
      const boost::optional<iroha::protocol::ToriiResponse> &resp,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> &response_writer)
      const {
    if (resp) {
      if (isFinalStatus(resp->tx_status())) {
        log_->debug("Transaction {} in service cache and final",
                    iroha::bytestringToHexstring(resp->tx_hash()));
        response_writer.WriteLast(*resp, grpc::WriteOptions());
        return true;
      }
      log_->debug("Transaction {} in service cache and not final",
                  iroha::bytestringToHexstring(resp->tx_hash()));
      response_writer.Write(*resp);
    }
    return false;
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
