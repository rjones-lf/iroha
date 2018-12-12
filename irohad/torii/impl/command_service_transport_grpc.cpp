/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/default_constructible_unary_fn.hpp"  // non-copyable value workaround

#include "torii/impl/command_service_transport_grpc.hpp"

#include <iterator>

#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "common/timeout.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"

namespace torii {

  CommandServiceTransportGrpc::CommandServiceTransportGrpc(
      std::shared_ptr<CommandService> command_service,
      std::shared_ptr<iroha::torii::StatusBus> status_bus,
      std::chrono::milliseconds initial_timeout,
      std::chrono::milliseconds nonfinal_timeout,
      std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory,
      std::shared_ptr<TransportFactoryType> transaction_factory,
      std::shared_ptr<shared_model::interface::TransactionBatchParser>
          batch_parser,
      std::shared_ptr<shared_model::interface::TransactionBatchFactory>
          transaction_batch_factory)
      : command_service_(std::move(command_service)),
        status_bus_(std::move(status_bus)),
        initial_timeout_(initial_timeout),
        nonfinal_timeout_(nonfinal_timeout),
        status_factory_(std::move(status_factory)),
        transaction_factory_(std::move(transaction_factory)),
        batch_parser_(std::move(batch_parser)),
        batch_factory_(std::move(transaction_batch_factory)),
        log_(logger::log("CommandServiceTransportGrpc")) {}

  grpc::Status CommandServiceTransportGrpc::Torii(
      grpc::ServerContext *context,
      const iroha::protocol::Transaction *request,
      google::protobuf::Empty *response) {
    iroha::protocol::TxList single_tx_list;
    *single_tx_list.add_transactions() = *request;
    return ListTorii(context, &single_tx_list, response);
  }

  namespace {
    /**
     * Form an error message, which is to be shared between all transactions, if
     * there are several of them, or individual message, if there's only one
     * @param tx_hashes is non empty hash list to form error message from
     * @param error of those tx(s)
     * @return message
     */
    std::string formErrorMessage(
        const std::vector<shared_model::crypto::Hash> &tx_hashes,
        const std::string &error) {
      if (tx_hashes.size() == 1) {
        return (boost::format("Stateless invalid tx, error: %s, hash: %s")
                % error % tx_hashes[0].hex())
            .str();
      }

      std::string folded_hashes =
          std::accumulate(std::next(tx_hashes.begin()),
                          tx_hashes.end(),
                          tx_hashes[0].hex(),
                          [](auto &&acc, const auto &h) -> std::string {
                            return acc + ", " + h.hex();
                          });

      return (boost::format(
                  "Stateless invalid tx in transaction sequence, error: %s\n"
                  "Hash list: [%s]")
              % error % folded_hashes)
          .str();
    }
  }  // namespace

  shared_model::interface::types::SharedTxsCollectionType
  CommandServiceTransportGrpc::deserializeTransactions(
      const iroha::protocol::TxList *request) {
    return boost::copy_range<
        shared_model::interface::types::SharedTxsCollectionType>(
        request->transactions()
        | boost::adaptors::transformed(
              [&](const auto &tx) { return transaction_factory_->build(tx); })
        | boost::adaptors::filtered([&](const auto &result) {
            return result.match(
                [](const iroha::expected::Value<
                    std::unique_ptr<shared_model::interface::Transaction>> &) {
                  return true;
                },
                [&](const iroha::expected::Error<TransportFactoryType::Error>
                        &error) {
                  status_bus_->publish(status_factory_->makeStatelessFail(
                      error.error.hash,
                      shared_model::interface::TxStatusFactory::
                          TransactionError{error.error.error, 0, 0}));
                  return false;
                });
          })
        | boost::adaptors::transformed([&](auto result) {
            return std::move(
                       boost::get<iroha::expected::ValueOf<decltype(result)>>(
                           result))
                .value;
          }));
  }

  grpc::Status CommandServiceTransportGrpc::ListTorii(
      grpc::ServerContext *context,
      const iroha::protocol::TxList *request,
      google::protobuf::Empty *response) {
    auto transactions = deserializeTransactions(request);

    auto batches = batch_parser_->parseBatches(transactions);

    for (auto &batch : batches) {
      batch_factory_->createTransactionBatch(batch).match(
          [&](iroha::expected::Value<std::unique_ptr<
                  shared_model::interface::TransactionBatch>> &value) {
            this->command_service_->handleTransactionBatch(
                std::move(value).value);
          },
          [&](iroha::expected::Error<std::string> &error) {
            std::vector<shared_model::crypto::Hash> hashes;

            std::transform(batch.begin(),
                           batch.end(),
                           std::back_inserter(hashes),
                           [](const auto &tx) { return tx->hash(); });

            auto error_msg = formErrorMessage(hashes, error.error);
            // set error response for each transaction in a batch candidate
            std::for_each(
                hashes.begin(), hashes.end(), [this, &error_msg](auto &hash) {
                  status_bus_->publish(status_factory_->makeStatelessFail(
                      hash,
                      shared_model::interface::TxStatusFactory::
                          TransactionError{error_msg, 0, 0}));
                });
          });
    }

    return grpc::Status::OK;
  }

  grpc::Status CommandServiceTransportGrpc::Status(
      grpc::ServerContext *context,
      const iroha::protocol::TxStatusRequest *request,
      iroha::protocol::ToriiResponse *response) {
    *response =
        std::static_pointer_cast<shared_model::proto::TransactionResponse>(
            command_service_->getStatus(
                shared_model::crypto::Hash(request->tx_hash())))
            ->getTransport();
    return grpc::Status::OK;
  }

  namespace {
    void handleEvents(rxcpp::composite_subscription &subscription,
                      rxcpp::schedulers::run_loop &run_loop) {
      while (subscription.is_subscribed() or not run_loop.empty()) {
        run_loop.dispatch();
      }
    }
  }  // namespace

  grpc::Status CommandServiceTransportGrpc::StatusStream(
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

    command_service_
        ->getStatusStream(hash)
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
}  // namespace torii
