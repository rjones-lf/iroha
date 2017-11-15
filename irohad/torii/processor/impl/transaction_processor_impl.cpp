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

#include <iostream>
#include <utility>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "crypto/hash.hpp"
#include "endpoint.pb.h"
#include "model/transaction_response.hpp"

namespace iroha {
  namespace torii {

    using model::TransactionResponse;
    using Status = model::TransactionResponse::Status;
    using network::PeerCommunicationService;
    using validation::StatelessValidator;

    TransactionProcessorImpl::TransactionProcessorImpl(
        std::shared_ptr<PeerCommunicationService> pcs,
        std::shared_ptr<StatelessValidator> validator,
        std::shared_ptr<MstProcessor> mst_processor)
        : pcs_(std::move(pcs)),
          validator_(std::move(validator)),
          mst_processor_(std::move(mst_processor)) {
      log_ = logger::log("TxProcessor");

      // insert all txs from proposal to proposal set
      pcs_->on_proposal().subscribe([this](model::Proposal proposal) {
        for (const auto &tx : proposal.transactions) {
          proposal_set_.insert(hash(tx).to_string());
          notify(hash(tx).to_string(), Status::STATELESS_VALIDATION_SUCCESS);
        }
      });

      // move commited txs from proposal to candidate map
      pcs_->on_commit().subscribe([this](auto &&blocks) {
        blocks.subscribe(
            // on next..
            [this](auto &&block) {
              const auto in_proposal = [this](const auto &tx) {
                return this->proposal_set_.count(hash(tx).to_string());
              };
              boost::for_each(
                  block.transactions | boost::adaptors::filtered(in_proposal),
                  [this](auto &&t) {
                    const auto &h = hash(t).to_string();
                    this->proposal_set_.erase(h);
                    this->candidate_set_.insert(h);
                    this->notify(h, Status::STATEFUL_VALIDATION_SUCCESS);
                  });
            },
            // on complete
            [this]() {
              boost::for_each(this->proposal_set_, [this](auto &&t) {
                return this->notify(t, Status::STATEFUL_VALIDATION_FAILED);
              });
              this->proposal_set_.clear();
              boost::for_each(this->candidate_set_, [this](auto &&t) {
                return this->notify(t, Status::COMMITTED);
              });
              this->candidate_set_.clear();
            });
      });

      mst_processor_->onPreparedTransactions().subscribe(
          [this](auto &&tx) { return this->transactionHandle(tx); });
      mst_processor_->onExpiredTransactions().subscribe([this](auto &&tx) {
        return this->notify(hash(*tx).to_string(), Status::EXPIRED);
      });
    }

    void TransactionProcessorImpl::transactionHandle(
        ConstRefTransaction transaction) {
      log_->info("handle transaction");
      const auto &h = hash(*transaction).to_string();

      if (!validator_->validate(*transaction)) {
        log_->info("stateless validation failed");
        notify(h, Status::STATELESS_VALIDATION_FAILED);
        return;
      }

      if (transaction->signatures.size() < transaction->quorum) {
        log_->info("waiting for quorum signatures");
        mst_processor_->propagateTransaction(transaction);
      } else {
        log_->info("propagating tx");
        pcs_->propagate_transaction(transaction);
      }
      notify(h, Status::STATELESS_VALIDATION_SUCCESS);
    }

    rxcpp::observable<TxResponse>
    TransactionProcessorImpl::transactionNotifier() {
      return notifier_.get_observable();
    }

    void TransactionProcessorImpl::notify(const std::string &hash, Status s) {
      notifier_.get_subscriber().on_next(
          std::make_shared<TransactionResponse>(hash, s));
    }
  }  // namespace torii
}  // namespace iroha
