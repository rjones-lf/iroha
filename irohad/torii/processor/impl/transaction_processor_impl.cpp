/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include "torii/processor/transaction_processor_impl.hpp"

#include <boost/format.hpp>

#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "validation/stateful_validator_common.hpp"

namespace iroha {
  namespace torii {

    using network::PeerCommunicationService;

    static std::string composeErrorMessage(
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

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<MstProcessor> mst_processor)
        : pcs_(std::move(pcs)), mst_processor_(std::move(mst_processor)) {
      log_ = logger::log("TxProcessor");

      // notify about stateless success
      pcs_->on_proposal().subscribe([this](auto model_proposal) {
        for (const auto &tx : model_proposal->transactions()) {
          const auto &hash = tx.hash();
          log_->info("on proposal stateless success: {}", hash.hex());
          // different on_next() calls (this one and below) can happen in
          // different threads and we don't expect emitting them concurrently
          std::lock_guard<std::mutex> lock(notifier_mutex_);
          notifier_.get_subscriber().on_next(
              shared_model::builder::DefaultTransactionStatusBuilder()
                  .statelessValidationSuccess()
                  .txHash(hash)
                  .build());
        }
      });

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
              notifier_.get_subscriber().on_next(
                  shared_model::builder::DefaultTransactionStatusBuilder()
                      .statefulValidationFailed()
                      .txHash(tx_error.second)
                      .errorMsg(error_msg)
                      .build());
            }
            // notify about success txs
            for (const auto &successful_tx :
                 proposal_and_errors->first->transactions()) {
              log_->info("on stateful validation success: {}",
                         successful_tx.hash().hex());
              notifier_.get_subscriber().on_next(
                  shared_model::builder::DefaultTransactionStatusBuilder()
                      .statefulValidationSuccess()
                      .txHash(successful_tx.hash())
                      .build());
            }
          });

      // commit transactions
      pcs_->on_commit().subscribe([this](Commit blocks) {
        blocks.subscribe(
            // on next
            [this](auto model_block) {
              current_txs_hashes_.reserve(model_block->transactions().size());
              std::transform(model_block->transactions().begin(),
                             model_block->transactions().end(),
                             std::back_inserter(current_txs_hashes_),
                             [](const auto &tx) { return tx.hash(); });
            },
            // on complete
            [this]() {
              if (current_txs_hashes_.empty()) {
                log_->info("there are no transactions to be committed");
              } else {
                std::lock_guard<std::mutex> lock(notifier_mutex_);
                for (const auto &tx_hash : current_txs_hashes_) {
                  log_->info("on commit committed: {}", tx_hash.hex());
                  notifier_.get_subscriber().on_next(
                      shared_model::builder::DefaultTransactionStatusBuilder()
                          .committed()
                          .txHash(tx_hash)
                          .build());
                }
              }
              current_txs_hashes_.clear();
            });
      });

      mst_processor_->onPreparedTransactions().subscribe([this](auto &&tx) {
        log_->info("MST tx prepared");
        return this->pcs_->propagate_transaction(tx);
      });
      mst_processor_->onExpiredTransactions().subscribe([this](auto &&tx) {
        log_->info("MST tx expired");
        std::lock_guard<std::mutex> lock(notifier_mutex_);
        this->notifier_.get_subscriber().on_next(
            shared_model::builder::DefaultTransactionStatusBuilder()
                .mstExpired()
                .txHash(tx->hash())
                .build());
        ;
      });
    }

    void TransactionProcessorImpl::transactionHandle(
        std::shared_ptr<shared_model::interface::Transaction> transaction) {
      log_->info("handle transaction");
      if (boost::size(transaction->signatures()) < transaction->quorum()) {
        log_->info("waiting for quorum signatures");
        mst_processor_->propagateTransaction(transaction);
        return;
      }

      log_->info("propagating tx");
      pcs_->propagate_transaction(transaction);
    }

    rxcpp::observable<
        std::shared_ptr<shared_model::interface::TransactionResponse>>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
