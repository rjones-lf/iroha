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
        const auto tx_hash = tx_hash_and_error.tx_hash.hex();
        const auto &cmd_error = tx_hash_and_error.error;
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
            status_factory,
        logger::Logger log)
        : pcs_(std::move(pcs)),
          mst_processor_(std::move(mst_processor)),
          status_bus_(std::move(status_bus)),
          status_factory_(std::move(status_factory)),
          log_(std::move(log)) {
      // process stateful validation results
      pcs_->onVerifiedProposal().subscribe(
          [this](const simulator::VerifiedProposalCreatorEvent &event) {
            if (not event.verified_proposal_result) {
              return;
            }

            const auto &proposal_and_errors = getVerifiedProposalUnsafe(event);

            // notify about failed txs
            const auto &errors = proposal_and_errors->rejected_transactions;
            std::lock_guard<std::mutex> lock(notifier_mutex_);
            for (const auto &tx_error : errors) {
              log_->info(composeErrorMessage(tx_error));
              this->publishStatus(TxStatusType::kStatefulFailed,
                                  tx_error.tx_hash,
                                  tx_error.error);
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
            bool has_at_least_one_committed = false;
            sync_event.synced_blocks.subscribe(
                // on next
                [this, &has_at_least_one_committed](auto model_block) {
                  std::lock_guard<std::mutex> lock(notifier_mutex_);
                  for (const auto &tx : model_block->transactions()) {
                    const auto &hash = tx.hash();
                    log_->info("on commit committed: {}", hash.hex());
                    this->publishStatus(TxStatusType::kCommitted, hash);
                    has_at_least_one_committed = true;
                  }
                  for (const auto &rejected_tx_hash :
                       model_block->rejected_transactions_hashes()) {
                    log_->info("on commit rejected: {}",
                               rejected_tx_hash.hex());
                    this->publishStatus(TxStatusType::kRejected,
                                        rejected_tx_hash);
                  }
                },
                // on complete
                [this, &has_at_least_one_committed] {
                  if (not has_at_least_one_committed) {
                    log_->info("there are no transactions to be committed");
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
        log_->info("MST batch {} is expired", batch->reducedHash());
        for (auto &&tx : batch->transactions()) {
          this->publishStatus(TxStatusType::kMstExpired, tx->hash());
        }
      });
    }

    void TransactionProcessorImpl::batchHandle(
        std::shared_ptr<shared_model::interface::TransactionBatch>
            transaction_batch) const {
      log_->info("handle batch");
      if (transaction_batch->hasAllSignatures()
          and not mst_processor_->batchInStorage(transaction_batch)) {
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
        case TxStatusType::kRejected: {
          status_bus_->publish(status_factory_->makeRejected(hash, tx_error));
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
