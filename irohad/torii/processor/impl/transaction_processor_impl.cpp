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
#include <endpoint.pb.h>
#include <iostream>
#include <utility>
#include "backend/protobuf/from_old_model.hpp"
#include "backend/protobuf/transaction.hpp"
#include "model/sha3_hash.hpp"
#include "model/transaction_response.hpp"

namespace iroha {
  namespace torii {

    using model::TransactionResponse;
    using network::PeerCommunicationService;
    using validation::StatelessValidator;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs)
        : pcs_(std::move(pcs)) {
      log_ = logger::log("TxProcessor");

      // insert all txs from proposal to proposal set
      pcs_->on_proposal().subscribe([this](auto model_proposal) {
        auto proposal =
            std::unique_ptr<model::Proposal>(model_proposal->makeOldModel());
        for (const auto &tx : proposal->transactions) {
          proposal_set_.insert(hash(tx).to_string());
          TransactionResponse response;
          response.tx_hash = hash(tx).to_string();
          response.current_status =
              TransactionResponse::STATELESS_VALIDATION_SUCCESS;
          notifier_.get_subscriber().on_next(
              std::make_shared<model::TransactionResponse>(response));
        }
      });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe([this](Commit blocks) {
        blocks.subscribe(
            // on next..
            [this](auto model_block) {
              std::unique_ptr<model::Block> block(model_block->makeOldModel());
              for (const auto &tx : block->transactions) {
                if (this->proposal_set_.count(hash(tx).to_string())) {
                  proposal_set_.erase(hash(tx).to_string());
                  candidate_set_.insert(hash(tx).to_string());
                  TransactionResponse response;
                  response.tx_hash = hash(tx).to_string();
                  response.current_status =
                      model::TransactionResponse::STATEFUL_VALIDATION_SUCCESS;
                  notifier_.get_subscriber().on_next(
                      std::make_shared<model::TransactionResponse>(response));
                }
              }
            },
            // on complete
            [this]() {
              for (auto tx_hash : proposal_set_) {
                TransactionResponse response;
                response.tx_hash = tx_hash;
                response.current_status =
                    TransactionResponse::STATEFUL_VALIDATION_FAILED;
                notifier_.get_subscriber().on_next(
                    std::make_shared<model::TransactionResponse>(response));
              }
              proposal_set_.clear();

              for (auto tx_hash : candidate_set_) {
                TransactionResponse response;
                response.tx_hash = tx_hash;
                response.current_status = TransactionResponse::COMMITTED;
                notifier_.get_subscriber().on_next(
                    std::make_shared<model::TransactionResponse>(response));
              }
              candidate_set_.clear();
            });
      });
    }

    void TransactionProcessorImpl::transactionHandle(
        std::shared_ptr<model::Transaction> transaction) {
      log_->info("handle transaction");
      model::TransactionResponse response;
      response.tx_hash = hash(*transaction).to_string();
      response.current_status =
          model::TransactionResponse::Status::STATELESS_VALIDATION_SUCCESS;

      pcs_->propagate_transaction(
          std::make_shared<shared_model::proto::Transaction>(
              shared_model::proto::from_old(*transaction)));

      log_->info("stateless validated");
      notifier_.get_subscriber().on_next(
          std::make_shared<model::TransactionResponse>(response));
    }

    rxcpp::observable<std::shared_ptr<model::TransactionResponse>>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
