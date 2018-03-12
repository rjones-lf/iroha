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
#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

namespace iroha {
  namespace torii {

    using model::TransactionResponse;
    using network::PeerCommunicationService;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs)
        : pcs_(std::move(pcs)) {
      log_ = logger::log("TxProcessor");

      // insert all txs from proposal to proposal set
      pcs_->on_proposal().subscribe([this](auto model_proposal) {
        for (const auto &tx : model_proposal->transactions()) {
          auto hash = tx->hash();
          proposal_set_.insert(hash);
          notifier_.get_subscriber().on_next(
              status_builder_.statelessValidationSuccess()
                  .txHash(hash)
                  .build());
        }
      });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe([this](Commit blocks) {
        blocks.subscribe(
            // on next..
            [this](auto model_block) {
              for (const auto &tx : model_block->transactions()) {
                auto hash = tx->hash();
                if (this->proposal_set_.find(hash) != proposal_set_.end()) {
                  proposal_set_.erase(hash);
                  candidate_set_.insert(hash);
                  notifier_.get_subscriber().on_next(
                      status_builder_.statefulValidationSuccess()
                          .txHash(hash)
                          .build());
                }
              }
            },
            // on complete
            [this]() {
              for (auto& tx_hash : proposal_set_) {
                notifier_.get_subscriber().on_next(
                    status_builder_.statefulValidationFailed()
                        .txHash(tx_hash)
                        .build());
              }
              proposal_set_.clear();

              for (auto tx_hash : candidate_set_) {
                notifier_.get_subscriber().on_next(
                    status_builder_.committed().txHash(tx_hash).build());
              }
              candidate_set_.clear();
            });
      });
    }

    void TransactionProcessorImpl::transactionHandle(
        std::shared_ptr<shared_model::interface::Transaction> transaction) {
      log_->info("handle transaction");

      pcs_->propagate_transaction(transaction);
    }

    rxcpp::observable<
        std::shared_ptr<shared_model::interface::TransactionResponse>>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
