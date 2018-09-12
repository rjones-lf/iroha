/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/impl/command_service_impl.hpp"

#include <thread>

#include "ametsuchi/block_query.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "common/byteutils.hpp"
#include "common/is_any.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/default_validator.hpp"

namespace torii {

  CommandServiceImpl::CommandServiceImpl(
      std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
      std::shared_ptr<iroha::ametsuchi::Storage> storage,
      std::shared_ptr<iroha::torii::StatusBus> status_bus)
      : tx_processor_(std::move(tx_processor)),
        storage_(std::move(storage)),
        status_bus_(std::move(status_bus)),
        cache_(std::make_shared<CacheType>()),
        log_(logger::log("CommandServiceImpl")) {
    // Notifier for all clients
    status_bus_->statuses().subscribe([this](auto response) {
      // find response for this tx in cache; if status of received response
      // isn't "greater" than cached one, dismiss received one
      auto tx_hash = response->transactionHash();
      auto cached_tx_state = cache_->findItem(tx_hash);
      if (cached_tx_state
          and response->comparePriorities(**cached_tx_state)
              != shared_model::interface::TransactionResponse::
                     PrioritiesComparisonResult::kGreater) {
        return;
      }
      cache_->addItem(tx_hash, response);
    });
  }

  namespace {
    std::shared_ptr<shared_model::interface::TransactionResponse> makeResponse(
        const shared_model::crypto::Hash &h,
        const iroha::protocol::TxStatus &status,
        const std::string &error_msg) {
      iroha::protocol::ToriiResponse response;
      response.set_tx_hash(shared_model::crypto::toBinaryString(h));
      response.set_tx_status(status);
      response.set_error_message(error_msg);
      return std::static_pointer_cast<
          shared_model::interface::TransactionResponse>(
          std::make_shared<shared_model::proto::TransactionResponse>(
              std::move(response)));
    }
  }  // namespace

  void CommandServiceImpl::handleTransactionList(
      const shared_model::interface::TransactionSequence &tx_list) {
    for (const auto &batch : tx_list.batches()) {
      processBatch(batch);
    }
  }

  std::shared_ptr<shared_model::interface::TransactionResponse>
  CommandServiceImpl::getStatus(const shared_model::crypto::Hash &request) {
    auto cached = cache_->findItem(request);
    if (cached) {
      return cached.value();
    }

    const bool is_present = storage_->getBlockQuery()->hasTxWithHash(request);
    iroha::protocol::TxStatus status = is_present
        ? iroha::protocol::TxStatus::COMMITTED
        : iroha::protocol::TxStatus::NOT_RECEIVED;

    auto response = makeResponse(request, status, "");

    if (is_present) {
      cache_->addItem(request, response);
    } else {
      log_->warn("Asked non-existing tx: {}", request.hex());
    }

    return response;
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
  CommandServiceImpl::getStatusStream(const shared_model::crypto::Hash &hash) {
    using ResponsePtrType =
        std::shared_ptr<shared_model::interface::TransactionResponse>;
    auto initial_status = cache_->findItem(hash).value_or([&] {
      log_->debug("tx is not received: {}", hash.toString());
      return std::make_shared<shared_model::proto::TransactionResponse>(
          shared_model::proto::TransactionStatusBuilder()
              .txHash(hash)
              .notReceived()
              .build());
    }());
    return status_bus_
        ->statuses()
        // prepend initial status
        .start_with(initial_status)
        // select statuses with requested hash
        .filter(
            [&](auto response) { return response->transactionHash() == hash; })
        // successfully complete the observable if final status is received.
        // final status is included in the observable
        .template lift<ResponsePtrType>([](rxcpp::subscriber<ResponsePtrType>
                                               dest) {
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

  void CommandServiceImpl::pushStatus(
      const std::string &who,
      std::shared_ptr<shared_model::interface::TransactionResponse> response) {
    log_->debug(
        "{}: adding item to cache: {}, status {} ",
        who,
        response->transactionHash().hex(),
        iroha::protocol::TxStatus_Name(
            static_cast<iroha::protocol::TxStatus>(response->get().which())));
    status_bus_->publish(response);
  }

  void CommandServiceImpl::processBatch(
      const shared_model::interface::TransactionBatch &batch) {
    tx_processor_->batchHandle(batch);
    const auto &txs = batch.transactions();
    std::for_each(txs.begin(), txs.end(), [this](const auto &tx) {
      const auto &tx_hash = tx->hash();
      if (cache_->findItem(tx_hash) and tx->quorum() < 2) {
        log_->warn("Found transaction {} in cache, ignoring", tx_hash.hex());
        return;
      }

      this->pushStatus(
          "ToriiBatchProcessor",
          makeResponse(tx_hash,
                       iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS,
                       ""));
    });
  }

}  // namespace torii
