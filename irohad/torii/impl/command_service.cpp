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

#include "ametsuchi/block_query.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "builders/protobuf/transaction_sequence_builder.hpp"
#include "common/byteutils.hpp"
#include "common/is_any.hpp"
#include "common/timeout.hpp"
#include "common/types.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "validators/default_validator.hpp"

namespace torii {

  CommandService::CommandService(
      std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
      std::shared_ptr<iroha::ametsuchi::Storage> storage,
      std::shared_ptr<iroha::torii::StatusBus> status_bus,
      std::chrono::milliseconds initial_timeout,
      std::chrono::milliseconds nonfinal_timeout)
      : tx_processor_(tx_processor),
        storage_(storage),
        status_bus_(status_bus),
        initial_timeout_(initial_timeout),
        nonfinal_timeout_(nonfinal_timeout),
        cache_(std::make_shared<CacheType>()),
        log_(logger::log("CommandService")) {
    // Notifier for all clients
    status_bus_->statuses().subscribe([this](auto iroha_response) {
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

  namespace {
    iroha::protocol::ToriiResponse makeResponse(
        const shared_model::crypto::Hash &h,
        const iroha::protocol::TxStatus &status) {
      iroha::protocol::ToriiResponse response;
      response.set_tx_hash(shared_model::crypto::toBinaryString(h));
      response.set_tx_status(status);
      return response;
    }
  }  // namespace

  void CommandService::Torii(const iroha::protocol::Transaction &request) {
    shared_model::proto::TransportBuilder<
        shared_model::proto::Transaction,
        shared_model::validation::DefaultSignedTransactionValidator>()
        .build(request)
        .match(
            [this](
                // success case
                iroha::expected::Value<shared_model::proto::Transaction>
                    &iroha_tx) {
              auto tx_hash = iroha_tx.value.hash();
              if (cache_->findItem(tx_hash) and iroha_tx.value.quorum() < 2) {
                log_->warn("Found transaction {} in cache, ignoring",
                           tx_hash.hex());
                return;
              }

              // TODO: 08/08/2018 @muratovv remove duplication between Torii and
              // ListTorii IR-1583
              auto single_batch_result = shared_model::interface::
                  TransactionBatch::createTransactionBatch(
                      std::make_shared<shared_model::proto::Transaction>(
                          std::move(iroha_tx.value)),
                      shared_model::validation::
                          DefaultSignedTransactionValidator());
              single_batch_result.match(
                  [this](const iroha::expected::Value<
                         shared_model::interface::TransactionBatch> &value) {
                    tx_processor_->batchHandle(value.value);
                  },
                  [this](const auto &err) {
                    log_->warn("Transaction can't transformed to batch: {}",
                               err.error);
                  });

              this->pushStatus(
                  "Torii",
                  std::move(tx_hash),
                  makeResponse(
                      tx_hash,
                      iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS));
            },
            [this, &request](const auto &error) {
              // getting hash from invalid transaction
              auto blobPayload =
                  shared_model::proto::makeBlob(request.payload());
              auto tx_hash =
                  shared_model::crypto::DefaultHashProvider::makeHash(
                      blobPayload);
              log_->warn("Stateless invalid tx: {}, hash: {}",
                         error.error,
                         tx_hash.hex());

              // setting response
              auto response = makeResponse(
                  tx_hash,
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
              response.set_error_message(std::move(error.error));

              this->pushStatus(
                  "Torii", std::move(tx_hash), std::move(response));
            });
  }

  void CommandService::ListTorii(const iroha::protocol::TxList &tx_list) {
    shared_model::proto::TransportBuilder<
        shared_model::interface::TransactionSequence,
        shared_model::validation::DefaultUnsignedTransactionsValidator>()
        .build(tx_list)
        .match(
            [this](
                // success case
                iroha::expected::Value<
                    shared_model::interface::TransactionSequence>
                    &tx_sequence) {
              for (const auto &batch : tx_sequence.value.batches()) {
                tx_processor_->batchHandle(batch);
                const auto &txs = batch.transactions();
                std::for_each(txs.begin(), txs.end(), [this](const auto &tx) {
                  auto tx_hash = tx->hash();
                  if (cache_->findItem(tx_hash) and tx->quorum() < 2) {
                    log_->warn("Found transaction {} in cache, ignoring",
                               tx_hash.hex());
                    return;
                  }

                  this->pushStatus(
                      "ToriiList",
                      std::move(tx_hash),
                      makeResponse(tx_hash,
                                   iroha::protocol::TxStatus::
                                       STATELESS_VALIDATION_SUCCESS));
                });
              }
            },
            [this, &tx_list](auto &error) {
              auto &txs = tx_list.transactions();
              if (txs.empty()) {
                log_->warn("Received empty transaction sequence");
                return;
              }
              // form an error message, shared between all txs in a sequence
              auto first_tx_blob =
                  shared_model::proto::makeBlob(txs[0].payload());
              auto first_tx_hash =
                  shared_model::crypto::DefaultHashProvider::makeHash(
                      first_tx_blob);
              auto last_tx_blob =
                  shared_model::proto::makeBlob(txs[txs.size() - 1].payload());
              auto last_tx_hash =
                  shared_model::crypto::DefaultHashProvider::makeHash(
                      last_tx_blob);
              auto sequence_error =
                  "Stateless invalid tx in transaction sequence, beginning "
                  "with tx : "
                  + first_tx_hash.hex() + " and ending with tx "
                  + last_tx_hash.hex() + ". Error is: " + error.error;

              // set error response for each transaction in a sequence
              std::for_each(
                  txs.begin(), txs.end(), [this, &sequence_error](auto &tx) {
                    auto hash =
                        shared_model::crypto::DefaultHashProvider::makeHash(
                            shared_model::proto::makeBlob(tx.payload()));

                    auto response = makeResponse(
                        hash,
                        iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
                    response.set_error_message(sequence_error);

                    this->pushStatus(
                        "ToriiList", std::move(hash), std::move(response));
                  });
            });
  }

  grpc::Status CommandService::Torii(
      grpc::ServerContext *context,
      const iroha::protocol::Transaction *request,
      google::protobuf::Empty *response) {
    Torii(*request);
    return grpc::Status::OK;
  }

  grpc::Status CommandService::ListTorii(grpc::ServerContext *context,
                                         const iroha::protocol::TxList *request,
                                         google::protobuf::Empty *response) {
    ListTorii(*request);
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
      auto hash = shared_model::crypto::Hash(request.tx_hash());
      if (storage_->getBlockQuery()->hasTxWithHash(hash)) {
        response.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
        cache_->addItem(std::move(hash), response);
      } else {
        log_->warn("Asked non-existing tx: {}",
                   iroha::bytestringToHexstring(request.tx_hash()));
        response.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      }
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
    using ResponsePtrType =
        std::shared_ptr<shared_model::interface::TransactionResponse>;
    ResponsePtrType initial_status =
        clone(shared_model::proto::TransactionResponse(
            cache_->findItem(hash).value_or([&] {
              log_->debug("tx not received: {}", hash.toString());
              return shared_model::proto::TransactionStatusBuilder()
                  .txHash(hash)
                  .notReceived()
                  .build()
                  .getTransport();
            }())));
    return status_bus_
        ->statuses()
        // prepend initial status
        .start_with(initial_status)
        // select statuses with requested hash
        .filter(
            [&](auto response) { return response->transactionHash() == hash; })
        // successfully complete the observable if final status is received.
        // final status is included in the observable
        .lift<ResponsePtrType>([](rxcpp::subscriber<ResponsePtrType> dest) {
          return rxcpp::make_subscriber<ResponsePtrType>(
              dest, [=](ResponsePtrType response) {
                dest.on_next(response);
                iroha::visit_in_place(
                    response->get(),
                    [dest](const auto &resp)
                        -> std::enable_if_t<FinalStatusValue<decltype(resp)>> {
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

    static auto client_id_format = boost::format("Peer: '%s', %s");
    std::string client_id =
        (client_id_format % context->peer() % hash.toString()).str();

    StatusStream(hash)
        // convert to transport objects
        .map([&](auto response) {
          log_->debug("mapped {}, {}", response->toString(), client_id);
          return std::static_pointer_cast<
                     shared_model::proto::TransactionResponse>(response)
              ->getTransport();
        })
        // set a corresponding observable timeout based on status value
        .lift<iroha::protocol::ToriiResponse>(
            iroha::makeTimeout<iroha::protocol::ToriiResponse>(
                [&](const auto &response) {
                  return response.tx_status()
                          == iroha::protocol::TxStatus::NOT_RECEIVED
                      ? initial_timeout_
                      : nonfinal_timeout_;
                },
                current_thread))
        // complete the observable if client is disconnected
        .take_while([=](const auto &) {
          auto is_cancelled = context->IsCancelled();
          if (is_cancelled) {
            log_->debug("client unsubscribed, {}", client_id);
          }
          return not is_cancelled;
        })
        .subscribe(subscription,
                   [&](iroha::protocol::ToriiResponse response) {
                     if (response_writer->Write(response)) {
                       log_->debug("status written, {}", client_id);
                     }
                   },
                   [&](std::exception_ptr ep) {
                     log_->debug("processing timeout, {}", client_id);
                   },
                   [&] { log_->debug("stream done, {}", client_id); });

    // run loop while subscription is active or there are pending events in
    // the queue
    handleEvents(subscription, rl);

    log_->debug("status stream done, {}", client_id);
    return grpc::Status::OK;
  }

  void CommandService::handleEvents(rxcpp::composite_subscription &subscription,
                                    rxcpp::schedulers::run_loop &run_loop) {
    while (subscription.is_subscribed() or not run_loop.empty()) {
      run_loop.dispatch();
    }
  }

  void CommandService::pushStatus(
      const std::string &who,
      const shared_model::crypto::Hash &hash,
      const iroha::protocol::ToriiResponse &response) {
    log_->debug("{}: adding item to cache: {}, status {} ",
                who,
                hash.hex(),
                iroha::protocol::TxStatus_Name(response.tx_status()));
    status_bus_->publish(
        std::make_shared<shared_model::proto::TransactionResponse>(
            std::move(response)));
  }

}  // namespace torii
