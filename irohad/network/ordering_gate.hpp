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

#ifndef IROHA_ORDERING_SERVICE_HPP
#define IROHA_ORDERING_SERVICE_HPP

#include <rxcpp/rx-observable.hpp>
#include "model/proposal.hpp"
#include "model/transaction.hpp"

namespace iroha {
  namespace network {

    /**
     * Ordering gate provide interface with network transaction order
     */
    class OrderingGate {
     public:
      /**
       * Propagate a signed transaction for further processing
       * @param transaction
       */
      virtual void propagateTransaction(
          std::shared_ptr<const model::Transaction> transaction) = 0;

      /**
       * Return observable of all proposals in the consensus
       * @return observable with notifications
       */
      virtual rxcpp::observable<model::Proposal> on_proposal() = 0;

      virtual ~OrderingGate() = default;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_HPP
