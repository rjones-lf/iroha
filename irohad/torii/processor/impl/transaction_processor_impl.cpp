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

#include <torii/processor/transaction_processor_impl.hpp>
#include <model/tx_responses/stateless_response.hpp>

namespace iroha {
  namespace torii {

    using validation::StatelessValidator;
    using model::TransactionResponse;
    using network::PeerCommunicationService;

    TransactionProcessorImpl::TransactionProcessorImpl(
        PeerCommunicationService &pcs,
        const StatelessValidator &validator)
        : pcs_(pcs),
          validator_(validator) {
    }

    void TransactionProcessorImpl::transaction_handle(model::Transaction &transaction) {
      model::TransactionStatelessResponse response;
      response.transaction = transaction;
      response.passed = false;

      if (validator_.validate(transaction)) {
        response.passed = true;
        pcs_.propagate_transaction(transaction);
      }

      notifier_.get_subscriber().on_next(
          std::make_shared<model::TransactionStatelessResponse>(response));
    }

    rxcpp::observable<std::shared_ptr<model::TransactionResponse>>
    TransactionProcessorImpl::transaction_notifier() {
      return notifier_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
