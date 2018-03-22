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
#ifndef IROHA_ORDERING_GATE_TRANSPORT_H
#define IROHA_ORDERING_GATE_TRANSPORT_H

#include <memory>

namespace shared_model {
  namespace interface {
    class Transaction;
    class Proposal;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace network {

    /**
     * OrderingGateNotification is a base class for any ordering gate
     * implementation that want to be notified about incoming proposals
     */
    class OrderingGateNotification {
     public:
      /**
       * Callback on receiving proposal
       * @param proposal - proposal object itself
       */
      virtual void onProposal(
          std::shared_ptr<shared_model::interface::Proposal>) = 0;

      virtual ~OrderingGateNotification() = default;
    };

    /**
     * A generic transport interface for ordering gate, any output transaction
     * must go through this transport Moreover, it receives transaction and then
     * routes it to a subscriber
     */
    class OrderingGateTransport {
     public:
      /**
       * Subscribes a notification class to current transport
       * @param subscriber : A pointer to OrderingGateNotification, that needs
       * to be notified
       */
      virtual void subscribe(
          std::shared_ptr<OrderingGateNotification> subscriber) = 0;

      /**
       * Propagates transaction over network
       * @param transaction : transaction to be propagated
       */
      virtual void propagateTransaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) = 0;

      virtual ~OrderingGateTransport() = default;
    };

  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ORDERING_GATE_TRANSPORT_H
