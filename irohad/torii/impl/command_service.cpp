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

#include "torii/command_service.hpp"

#include <thread>

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"

#include "ametsuchi/block_query.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "common/byteutils.hpp"
#include "common/is_any.hpp"
#include "common/types.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "endpoint.pb.h"
#include "torii/impl/timeout.hpp"
#include "validators/default_validator.hpp"

using namespace std::chrono_literals;

namespace torii {

  CommandService::CommandService(
      std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
      std::shared_ptr<iroha::ametsuchi::Storage> storage,
      std::chrono::milliseconds proposal_delay)
      : tx_processor_(tx_processor),
        storage_(storage),
        proposal_delay_(proposal_delay),
        start_tx_processing_duration_(1s),
        cache_(std::make_shared<CacheType>()),
        // merge with mutex, since notifications can be made from different
        // threads
        responses_(tx_processor_->transactionNotifier().merge(
            rxcpp::serialize_one_worker(
                rxcpp::schedulers::make_current_thread()),
            notifier_.get_observable())),
        log_(logger::log("CommandService")) {
    // Notifier for all clients
    responses_.subscribe([this](auto iroha_response) {
      // find response for this tx in cache; if status of received response
      // isn't "greater" than cached one, dismiss received one
      auto proto_response =
          std::static_pointer_cast<shared_model::proto::TransactionResponse>(
              iroha_response);
      auto tx_hash = proto_response->transactionHash();
      auto cached_tx_state = cache_->findItem(tx_hash);
      if (cached_tx_state
          and proto_response->getTransport().tx_status()
              <= cached_tx_state->tx_status()) {
        return;
      }
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
    // transactions can be handled from multiple threads, therefore a lock is
    // required
    std::lock_guard<std::mutex> lock(notifier_mutex_);
    notifier_.get_subscriber().on_next(
        std::make_shared<shared_model::proto::TransactionResponse>(
            std::move(response)));
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
      if (storage_->getBlockQuery()->hasTxWithHash(
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

  /**
   * Statuses considered final for streaming. Observable stops value emission
   * after receiving a value of one of the following types
   * @tparam T concrete response type
   */
  template <typename T>
  constexpr bool FinalStatusValue =
      iroha::is_any<std::decay_t<T>,
                    shared_model::interface::StatelessFailedTxResponse,
                    shared_model::interface::StatefulFailedTxResponse,
                    shared_model::interface::CommittedTxResponse,
                    shared_model::interface::MstExpiredResponse>::value;

  rxcpp::observable<
      std::shared_ptr<shared_model::interface::TransactionResponse>>
  CommandService::StatusStream(const shared_model::crypto::Hash &hash) {
    std::shared_ptr<shared_model::interface::TransactionResponse>
        initial_status = clone(shared_model::proto::TransactionResponse(
            cache_->findItem(hash).value_or([&] {
              log_->debug("tx not received");
              return shared_model::proto::TransactionStatusBuilder()
                  .txHash(hash)
                  .notReceived()
                  .build()
                  .getTransport();
            }())));
    return responses_.start_with(initial_status)
        .filter(
            [&](auto response) { return response->transactionHash() == hash; })
        .lift<std::shared_ptr<shared_model::interface::TransactionResponse>>(
            [](rxcpp::subscriber<std::shared_ptr<
                   shared_model::interface::TransactionResponse>> dest) {
              return rxcpp::make_subscriber<std::shared_ptr<
                  shared_model::interface::TransactionResponse>>(
                  dest,
                  [=](std::shared_ptr<
                      shared_model::interface::TransactionResponse> response) {
                    dest.on_next(response);
                    iroha::visit_in_place(
                        response->get(),
                        [dest](const auto &resp)
                            -> std::enable_if_t<
                                FinalStatusValue<decltype(resp)>> {
                          dest.on_completed();
                        },
                        [](const auto &resp)
                            -> std::enable_if_t<
                                not FinalStatusValue<decltype(resp)>>{});
                  });
            });
  }

  grpc::Status CommandService::StatusStream(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      grpc::ServerWriter<iroha::protocol::ToriiResponse> *response_writer) {
    rxcpp::schedulers::run_loop rl;

    auto current_thread =
        rxcpp::observe_on_one_worker(rxcpp::schedulers::make_run_loop(rl));

    rxcpp::composite_subscription subscription;

    auto hash = shared_model::crypto::Hash(request->tx_hash());

    StatusStream(hash)
        .map([](auto response) {
          return std::static_pointer_cast<
                     shared_model::proto::TransactionResponse>(response)
              ->getTransport();
        })
        .lift<iroha::protocol::ToriiResponse>(
            makeTimeout<iroha::protocol::ToriiResponse>(
                [&](const auto &response) {
                  return response.tx_status()
                          == iroha::protocol::TxStatus::NOT_RECEIVED
                      ? start_tx_processing_duration_
                      : 2 * proposal_delay_;
                },
                current_thread))
        .take_while([=](const auto &) { return not context->IsCancelled(); })
        .subscribe(
            subscription,
            [&](iroha::protocol::ToriiResponse response) {
              log_->debug("writing status");
              response_writer->Write(response);
            },
            [&](std::exception_ptr ep) { log_->debug("processing timeout"); },
            [&] { log_->debug("stream done"); });

    // run loop while subscription is active or there are pending events in the
    // queue
    while (subscription.is_subscribed() or not rl.empty()) {
      rl.dispatch();
    }

    log_->debug("status stream done");
    return grpc::Status::OK;
  }

}  // namespace torii
