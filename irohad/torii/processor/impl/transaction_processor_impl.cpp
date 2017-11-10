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
#include <boost/range/join.hpp>
#include <iostream>
#include <utility>
#include "crypto/hash.hpp"
#include "endpoint.pb.h"
#include "model/transaction_response.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

namespace iroha {
  namespace torii {

    using model::TransactionResponse;
    using Status = model::TransactionResponse::Status;
    using network::PeerCommunicationService;
    using validation::StatelessValidator;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<StatelessValidator> validator,
        std::shared_ptr<MstProcessor> mst_proc)
        : pcs_(std::move(pcs)),
          validator_(std::move(validator)),
          mst_proc_(std::move(mst_proc)) {
      log_ = logger::log("TxProcessor");

      // insert all txs from proposal to proposal set
      pcs_->on_proposal().subscribe([this](model::Proposal proposal) {
        for (const auto &tx : proposal.transactions) {
          proposal_set_.insert(hash(tx).to_string());
          notifier_.get_subscriber().on_next(
              std::make_shared<TransactionResponse>(TransactionResponse{
                  hash(tx).to_string(), Status::STATELESS_VALIDATION_SUCCESS}));
        }
      });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe([this](auto blocks) {
        blocks.subscribe(
            // on next..
            [this](auto block) {
              static auto in_proposal = [this](const auto &tx) {
                return this->proposal_set_.count(hash(tx).to_string());
              };
              static auto notify_success = [this](const auto &tx) {
                auto h = hash(tx).to_string();
                this->proposal_set_.erase(h);
                this->candidate_set_.insert(h);
                this->notifier_.get_subscriber().on_next(
                    std::make_shared<TransactionResponse>(TransactionResponse{
                        h,
                        Status::STATEFUL_VALIDATION_SUCCESS,
                    }));
              };
              boost::for_each(
                  block.transactions | boost::adaptors::filtered(in_proposal),
                  notify_success);
            },
            // on complete
            [this]() {
              for (const auto &tx_hash :
                   boost::join(this->proposal_set_, this->candidate_set_)) {
                this->notifier_.get_subscriber().on_next(
                    std::make_shared<TransactionResponse>(TransactionResponse{
                        tx_hash, Status::STATEFUL_VALIDATION_FAILED}));
              }
              this->proposal_set_.clear();
              this->candidate_set_.clear();
            });
      });
    }

    void TransactionProcessorImpl::transactionHandle(
        ConstRefTransaction transaction) {
      log_->info("handle transaction");
      TransactionResponse response{hash(*transaction).to_string(),
                                   Status::STATELESS_VALIDATION_FAILED};

      if (validator_->validate(*transaction)) {
        response.current_status = Status::STATELESS_VALIDATION_SUCCESS;
        pcs_->propagate_transaction(transaction);
      }
      log_->info(
          "stateless validation status: {}",
          response.current_status == Status::STATELESS_VALIDATION_SUCCESS);
      notifier_.get_subscriber().on_next(
          std::make_shared<TransactionResponse>(response));
    }

    rxcpp::observable<TxResponse>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
