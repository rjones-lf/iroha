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

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "torii/processor/transaction_processor_impl.hpp"
#include "validation/stateful_validator_common.hpp"

#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

namespace iroha {
  namespace torii {

    using network::PeerCommunicationService;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<MstProcessor> mst_processor)
        : pcs_(std::move(pcs)), mst_processor_(std::move(mst_processor)) {
      log_ = logger::log("TxProcessor");

      // notify about stateless success
      pcs_->on_proposal().subscribe([this](auto model_proposal) {
        for (const auto &tx : model_proposal->transactions()) {
          auto hash = tx.hash();
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
          [this](validation::VerifiedProposalAndErrors proposal_and_errors) {
            // notify about failed txs
            auto errors = proposal_and_errors.second;
            for (const auto &tx_error : errors) {
              log_->info("on stateful validation failed: {}",
                         tx_error.second.hex());
              std::lock_guard<std::mutex> lock(notifier_mutex_);
              notifier_.get_subscriber().on_next(
                  shared_model::builder::DefaultTransactionStatusBuilder()
                      .statefulValidationFailed()
                      .txHash(tx_error.second)
                      .errorMsg(tx_error.first)
                      .build());
            }
            // notify about success txs
            for (const auto &successful_tx : proposal_and_errors.first->transactions()) {
              log_->info("on stateful validation success: {}",
                         successful_tx.hash().hex());
              std::lock_guard<std::mutex> lock(notifier_mutex_);
              notifier_.get_subscriber().on_next(
                  shared_model::builder::DefaultTransactionStatusBuilder()
                      .statefulValidationSuccess()
                      .txHash(successful_tx.hash())
                      .build());
            }
            current_proposal_ = proposal_and_errors.first;
          });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe([this](Commit blocks) {
        blocks.subscribe(
            // on next
            [](auto model_block) {},
            // on complete
            [this]() {
              if (not current_proposal_) {
                log_->error("current proposal is empty");
              } else {
                for (auto &tx : current_proposal_->transactions()) {
                  log_->info("on commit committed: {}", tx.hash().hex());
                  std::lock_guard<std::mutex> lock(notifier_mutex_);
                  notifier_.get_subscriber().on_next(
                      shared_model::builder::DefaultTransactionStatusBuilder()
                          .committed()
                          .txHash(tx.hash())
                          .build());
                }
              }
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
