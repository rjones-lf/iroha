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

#include <ordering/ordering_service_stub.hpp>

namespace iroha {
  namespace ordering {

    using model::Transaction;
    using model::Proposal;

    void OrderingServiceStub::propagate_transaction(
        const model::Transaction &transaction) {
      std::vector<Transaction> transactions{transaction};
      Proposal proposal(transactions);
      proposals_.get_subscriber().on_next(proposal);
    }

    rxcpp::observable<model::Proposal> OrderingServiceStub::on_proposal() {
      return proposals_.get_observable();
    }
  }  // namespace ordering
}  // namespace iroha
