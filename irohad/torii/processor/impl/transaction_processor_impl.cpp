/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/processor/transaction_processor_impl.hpp"

#include <boost/format.hpp>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validation/stateful_validator_common.hpp"

namespace iroha {
  namespace torii {

    using network::PeerCommunicationService;

    namespace {
      std::string composeErrorMessage(
          const validation::TransactionError &tx_hash_and_error) {
        const auto tx_hash = tx_hash_and_error.first.hex();
        const auto &cmd_error = tx_hash_and_error.second;
        if (not cmd_error.tx_passed_initial_validation) {
          return (boost::format(
                      "Stateful validation error: transaction %s "
                      "did not pass initial verification: "
                      "checking '%s', error code '%d', query arguments: %s")
                  % tx_hash % cmd_error.name % cmd_error.error_code
                  % cmd_error.error_extra)
              .str();
        }
        return (boost::format(
                    "Stateful validation error in transaction %s: "
                    "command '%s' with index '%d' did not pass "
                    "verification with code '%d', query arguments: %s")
                % tx_hash % cmd_error.name % cmd_error.index
                % cmd_error.error_code % cmd_error.error_extra)
            .str();
      }
    }  // namespace

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<MstProcessor> mst_processor,
        std::shared_ptr<iroha::torii::StatusBus> status_bus,
        std::shared_ptr<shared_model::interface::TxStatusFactory>
            status_factory)
        : pcs_(std::move(pcs)),
          mst_processor_(std::move(mst_processor)),
          status_bus_(std::move(status_bus)),
          status_factory_(std::move(status_factory)),
          log_(logger::log("TxProcessor")) {
      // process stateful validation results
      pcs_->on_verified_proposal().subscribe(
          [this](std::shared_ptr<validation::VerifiedProposalAndErrors>
                     proposal_and_errors) {
            // notify about failed txs
            const auto &errors = proposal_and_errors->rejected_transactions;
            std::lock_guard<std::mutex> lock(notifier_mutex_);
            for (const auto &tx_error : errors) {
              log_->info(composeErrorMessage(tx_error));
              this->publishStatus(TxStatusType::kStatefulFailed,
                                  tx_error.first,
                                  tx_error.second);
            }
            // notify about success txs
            for (const auto &successful_tx :
                 proposal_and_errors->verified_proposal->transactions()) {
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

      mst_processor_->onStateUpdate().subscribe([this](auto &&state) {
        log_->info("MST state updated");
        for (auto &&batch : state->getBatches()) {
          for (auto &&tx : batch->transactions()) {
            this->publishStatus(TxStatusType::kMstPending, tx->hash());
          }
        }
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
        log_->info("propagating batch to MST");
        mst_processor_->propagateBatch(transaction_batch);
      }
    }

    void TransactionProcessorImpl::publishStatus(
        TxStatusType tx_status,
        const shared_model::crypto::Hash &hash,
        const validation::CommandError &cmd_error) const {
      auto tx_error = cmd_error.name.empty()
          ? shared_model::interface::TxStatusFactory::TransactionError{}
          : shared_model::interface::TxStatusFactory::TransactionError{
                cmd_error.name, cmd_error.index, cmd_error.error_code};
      switch (tx_status) {
        case TxStatusType::kStatelessFailed: {
          status_bus_->publish(
              status_factory_->makeStatelessFail(hash, tx_error));
          return;
        };
        case TxStatusType::kStatelessValid: {
          status_bus_->publish(
              status_factory_->makeStatelessValid(hash, tx_error));
          return;
        };
        case TxStatusType::kStatefulFailed: {
          status_bus_->publish(
              status_factory_->makeStatefulFail(hash, tx_error));
          return;
        };
        case TxStatusType::kStatefulValid: {
          status_bus_->publish(
              status_factory_->makeStatefulValid(hash, tx_error));
          return;
        };
        case TxStatusType::kCommitted: {
          status_bus_->publish(status_factory_->makeCommitted(hash, tx_error));
          return;
        };
        case TxStatusType::kMstExpired: {
          status_bus_->publish(status_factory_->makeMstExpired(hash, tx_error));
          return;
        };
        case TxStatusType::kNotReceived: {
          status_bus_->publish(
              status_factory_->makeNotReceived(hash, tx_error));
          return;
        };
        case TxStatusType::kMstPending: {
          status_bus_->publish(status_factory_->makeMstPending(hash, tx_error));
          return;
        };
        case TxStatusType::kEnoughSignaturesCollected: {
          status_bus_->publish(
              status_factory_->makeEnoughSignaturesCollected(hash, tx_error));
          return;
        };
      }
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
