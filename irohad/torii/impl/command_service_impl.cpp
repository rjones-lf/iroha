/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/impl/command_service_impl.hpp"

#include <thread>

#include "ametsuchi/block_query.hpp"
#include "common/byteutils.hpp"
#include "common/is_any.hpp"
#include "common/visitor.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/default_validator.hpp"

namespace torii {

  CommandServiceImpl::CommandServiceImpl(
      std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
      std::shared_ptr<iroha::ametsuchi::Storage> storage,
      std::shared_ptr<iroha::torii::StatusBus> status_bus,
      std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory)
      : tx_processor_(std::move(tx_processor)),
        storage_(std::move(storage)),
        status_bus_(std::move(status_bus)),
        cache_(std::make_shared<CacheType>()),
        status_factory_(std::move(status_factory)),
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

    if (is_present) {
      std::shared_ptr<shared_model::interface::TransactionResponse> response =
          status_factory_->makeCommitted(request, "");
      cache_->addItem(request, response);
      return response;
    } else {
      log_->warn("Asked non-existing tx: {}", request.hex());
      return status_factory_->makeNotReceived(request, "");
    }
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
      return status_factory_->makeNotReceived(hash, "");
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
    log_->debug("{}: adding item to cache: {}", who, response->toString());
    status_bus_->publish(response);
  }

  void CommandServiceImpl::processBatch(
      std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
    tx_processor_->batchHandle(batch);
    const auto &txs = batch->transactions();
    std::for_each(txs.begin(), txs.end(), [this](const auto &tx) {
      const auto &tx_hash = tx->hash();
      auto found = cache_->findItem(tx_hash);
      // StatlessValid status goes only after EnoughSignaturesCollectedResponse
      // So doesn't skip publishing status after it
      if (found
          and iroha::visit_in_place(
                  found.value()->get(),
                  [](const shared_model::interface::
                         EnoughSignaturesCollectedResponse &) { return false; },
                  [](auto &) { return true; })
          and tx->quorum() < 2) {
        log_->warn("Found transaction {} in cache, ignoring", tx_hash.hex());
        return;
      }

      this->pushStatus("ToriiBatchProcessor",
                       status_factory_->makeStatelessValid(tx_hash, ""));
    });
  }

}  // namespace torii
