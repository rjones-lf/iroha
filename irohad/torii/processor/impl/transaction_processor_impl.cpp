/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/processor/transaction_processor_impl.hpp"

#include <boost/format.hpp>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validation/stateful_validator_common.hpp"

namespace iroha {
  namespace torii {

    using network::PeerCommunicationService;

    namespace {
      std::string composeErrorMessage(
          const validation::TransactionError &tx_error) {
        if (not tx_error.first.tx_passed_initial_validation) {
          return (boost::format("Stateful validation error: transaction %s "
                                "did not pass initial verification: "
                                "checking '%s', error message '%s'")
                  % tx_error.second.hex() % tx_error.first.name
                  % tx_error.first.error)
              .str();
        }
        return (boost::format("Stateful validation error in transaction %s: "
                              "command '%s' with index '%d' did not pass "
                              "verification with error '%s'")
                % tx_error.second.hex() % tx_error.first.name
                % tx_error.first.index % tx_error.first.error)
            .str();
      }
    }  // namespace

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<MstProcessor> mst_processor,
        std::shared_ptr<iroha::torii::StatusBus> status_bus)
        : pcs_(std::move(pcs)),
          mst_processor_(std::move(mst_processor)),
          status_bus_(std::move(status_bus)),
          log_(logger::log("TxProcessor")) {
      // process stateful validation results
      pcs_->on_verified_proposal().subscribe(
          [this](std::shared_ptr<validation::VerifiedProposalAndErrors>
                     proposal_and_errors) {
            // notify about failed txs
            const auto &errors = proposal_and_errors->second;
            std::lock_guard<std::mutex> lock(notifier_mutex_);
            for (const auto &tx_error : errors) {
              auto error_msg = composeErrorMessage(tx_error);
              log_->info(error_msg);
              this->publishStatus(
                  TxStatusType::kStatefulFailed, tx_error.second, error_msg);
            }
            // notify about success txs
            for (const auto &successful_tx :
                 proposal_and_errors->first->transactions()) {
              log_->info("on stateful validation success: {}",
                         successful_tx.hash().hex());
              this->publishStatus(TxStatusType::kStatefulValid,
                                  successful_tx.hash());
            }
          });

      // commit transactions
      pcs_->on_commit().subscribe(
          [this](synchronizer::SynchronizationEvent sync_event) {
            sync_event.synced_blocks.subscribe(
                // on next
                [this](auto model_block) {
                  current_txs_hashes_.reserve(
                      model_block->transactions().size());
                  std::transform(model_block->transactions().begin(),
                                 model_block->transactions().end(),
                                 std::back_inserter(current_txs_hashes_),
                                 [](const auto &tx) { return tx.hash(); });
                },
                // on complete
                [this] {
                  if (current_txs_hashes_.empty()) {
                    log_->info("there are no transactions to be committed");
                  } else {
                    std::lock_guard<std::mutex> lock(notifier_mutex_);
                    for (const auto &tx_hash : current_txs_hashes_) {
                      log_->info("on commit committed: {}", tx_hash.hex());
                      this->publishStatus(TxStatusType::kCommitted, tx_hash);
                    }
                    current_txs_hashes_.clear();
                  }
                });
          });

      mst_processor_->onPreparedBatches().subscribe([this](auto &&batch) {
        log_->info("MST batch prepared");
        this->publishEnoughSignaturesStatus(batch->transactions());
        this->pcs_->propagate_batch(batch);
      });
      mst_processor_->onExpiredBatches().subscribe([this](auto &&batch) {
        log_->info("MST batch {} is expired", batch->reducedHash().toString());
        for (auto &&tx : batch->transactions()) {
          this->publishStatus(TxStatusType::kMstExpired, tx->hash());
        }
      });
    }

    void TransactionProcessorImpl::batchHandle(
        std::shared_ptr<shared_model::interface::TransactionBatch>
            transaction_batch) const {
      log_->info("handle batch");
      if (transaction_batch->hasAllSignatures()) {
        log_->info("propagating batch to PCS");
        this->publishEnoughSignaturesStatus(transaction_batch->transactions());
        pcs_->propagate_batch(transaction_batch);
      } else {
        for (const auto &tx : transaction_batch->transactions()) {
          this->publishStatus(TxStatusType::kMstPending, tx->hash());
        }
        log_->info("propagating batch to MST");
        mst_processor_->propagateBatch(transaction_batch);
      }
    }

    void TransactionProcessorImpl::publishStatus(
        TxStatusType tx_status,
        const shared_model::crypto::Hash &hash,
        const std::string &error) const {
      auto builder =
          shared_model::builder::DefaultTransactionStatusBuilder().txHash(hash);
      if (not error.empty()) {
        builder = builder.errorMsg(error);
      }
      switch (tx_status) {
        case TxStatusType::kStatelessFailed: {
          builder = builder.statelessValidationFailed();
          break;
        };
        case TxStatusType::kStatelessValid: {
          builder = builder.statelessValidationSuccess();
          break;
        };
        case TxStatusType::kStatefulFailed: {
          builder = builder.statefulValidationFailed();
          break;
        };
        case TxStatusType::kStatefulValid: {
          builder = builder.statefulValidationSuccess();
          break;
        };
        case TxStatusType::kCommitted: {
          builder = builder.committed();
          break;
        };
        case TxStatusType::kMstExpired: {
          builder = builder.mstExpired();
          break;
        };
        case TxStatusType::kNotReceived: {
          builder = builder.notReceived();
          break;
        };
        case TxStatusType::kMstPending: {
          builder = builder.mstPending();
          break;
        };
        case TxStatusType::kEnoughSignaturesCollected: {
          builder = builder.enoughSignaturesCollected();
          break;
        };
      }
      status_bus_->publish(builder.build());
    }

    void TransactionProcessorImpl::publishEnoughSignaturesStatus(
        const shared_model::interface::types::SharedTxsCollectionType &txs)
        const {
      for (const auto &tx : txs) {
        this->publishStatus(TxStatusType::kEnoughSignaturesCollected,
                            tx->hash());
      }
    }
  }  // namespace torii
}  // namespace iroha
