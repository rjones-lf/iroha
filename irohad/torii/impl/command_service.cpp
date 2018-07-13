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
#include "common/types.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "builders/protobuf/transaction_sequence_builder.hpp"
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
        log_(logger::log("CommandService")) {
    // Notifier for all clients
    tx_processor_->transactionNotifier().subscribe([this](auto iroha_response) {
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
    shared_model::proto::TransportBuilder<
        shared_model::proto::Transaction,
        shared_model::validation::DefaultSignableTransactionValidator>()
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

              // setting response
              iroha::protocol::ToriiResponse response;
              response.set_tx_hash(tx_hash.toString());
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);

              // Send transaction to iroha
              tx_processor_->transactionHandle(
                  std::make_shared<shared_model::proto::Transaction>(
                      std::move(iroha_tx.value)));

              addTxToCacheAndLog(
                  "Torii", std::move(tx_hash), std::move(response));
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
              iroha::protocol::ToriiResponse response;
              response.set_tx_hash(
                  shared_model::crypto::toBinaryString(tx_hash));
              response.set_tx_status(
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
              response.set_error_message(std::move(error.error));

              addTxToCacheAndLog(
                  "Torii", std::move(tx_hash), std::move(response));
            });
  }

  void CommandService::ListTorii(const iroha::protocol::TxList &tx_list) {
    shared_model::proto::TransportBuilder<
        shared_model::interface::TransactionSequence,
        shared_model::validation::DefaultBatchValidator>()
        .build(tx_list)
        .match(
            [this](
                // success case
                iroha::expected::Value<
                    shared_model::interface::TransactionSequence>
                    &tx_sequence) {
              std::for_each(
                  std::begin(tx_sequence.value.transactions()),
                  std::end(tx_sequence.value.transactions()),
                  [this](auto &tx) {
                    auto tx_hash = tx->hash();
                    if (cache_->findItem(tx_hash) and tx->quorum() < 2) {
                      log_->warn("Found transaction {} in cache, ignoring",
                                 tx_hash.hex());
                      return;
                    }

                    // setting response
                    iroha::protocol::ToriiResponse response;
                    response.set_tx_hash(tx_hash.toString());
                    response.set_tx_status(iroha::protocol::TxStatus::
                                               STATELESS_VALIDATION_SUCCESS);

                    // Send transaction to iroha
                    tx_processor_->transactionHandle(tx);

                    addTxToCacheAndLog(
                        "ToriiList", std::move(tx_hash), std::move(response));
                  });
            },
            [this, &tx_list](auto &error) {
              auto &txs = tx_list.transactions();
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
                  "Stateless invalid tx in transaction sequence: " + error.error
                  + ", hash of the first: " + first_tx_hash.hex()
                  + ", hash of the last: " + last_tx_hash.hex();

              // set error response for each transaction in a sequence
              std::for_each(
                  txs.begin(), txs.end(), [this, &sequence_error](auto &tx) {
                    auto hash =
                        shared_model::crypto::DefaultHashProvider::makeHash(
                            shared_model::proto::makeBlob(tx.payload()));

                    iroha::protocol::ToriiResponse response;
                    response.set_tx_hash(
                        shared_model::crypto::toBinaryString(hash));
                    response.set_tx_status(
                        iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
                    response.set_error_message(sequence_error);

                    addTxToCacheAndLog(
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
      if (storage_->getBlockQuery()->hasTxWithHash(
              shared_model::crypto::Hash(request.tx_hash()))) {
        response.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
      } else {
        log_->warn("Asked non-existing tx: {}",
                   iroha::bytestringToHexstring(request.tx_hash()));
        response.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      }
      addTxToCacheAndLog("Status", std::move(tx_hash), std::move(response));
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
            [&, finished](
                std::shared_ptr<shared_model::interface::TransactionResponse>
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

    if (not*finished) {
      resp = cache_->findItem(shared_model::crypto::Hash(request.tx_hash()));
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
        if (not*finished) {
          log_->debug("Transaction {} still not finished", request_hash->hex());
          resp =
              cache_->findItem(shared_model::crypto::Hash(request.tx_hash()));
          log_->debug("Status of tx {} in cache: {}",
                      request_hash->hex(),
                      resp->tx_status());
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

  void CommandService::addTxToCacheAndLog(
      const std::string &who,
      const shared_model::crypto::Hash &hash,
      const iroha::protocol::ToriiResponse &response) {
    log_->debug("{}: adding item to cache: {}, status {} ",
                who,
                hash.hex(),
                response.tx_status());
    cache_->addItem(hash, response);
  }

}  // namespace torii
