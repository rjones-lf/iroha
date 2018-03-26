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
#include "network/peer_communication_service.hpp"

namespace shared_model {
  namespace interface {
    class Transaction;
    class Proposal;
  }  // namespace interface
}  // namespace shared_model

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
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) = 0;

      /**
       * Return observable of all proposals in the consensus
       * @return observable with notifications
       */
      virtual rxcpp::observable<
          std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() = 0;

      /**
       * Set peer communication service for commit notification
       * @param pcs - const reference for PeerCommunicationService
       * design notes: pcs passed by const reference because of cyclic linking
       * between OG and PCS in the implementation. Same reasons to move the pcs
       * dependency not in ctor but make the setter method.
       */
      virtual void setPcs(const PeerCommunicationService &pcs) = 0;

      virtual ~OrderingGate() = default;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_HPP
