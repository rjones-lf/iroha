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
#ifndef IROHA_ORDERING_SERVICE_TRANSPORT_H
#define IROHA_ORDERING_SERVICE_TRANSPORT_H

#include <memory>
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"

namespace iroha {
  namespace network {

    /**
     * OrderingServiceNotification is a base class for any ordering service
     * implementation that want to be notified about incoming proposals
     */
    class OrderingServiceNotification {
     public:
      /**
       * Callback on receiving transaction
       * @param transaction - transaction object itself
       */
      virtual void onTransaction(
          std::shared_ptr<shared_model::interface::Transaction>
              transaction) = 0;

      virtual ~OrderingServiceNotification() = default;
    };

    /**
     * A generic transport interface for ordering service, any output
     * transaction must go through this transport Moreover, it receives
     * transaction and then routes it to a subscriber
     */
    class OrderingServiceTransport {
     public:
      /**
       * Subscribes a notification class to current transport
       * @param subscriber : A pointer to OrderingServiceNotification, that
       * needs to be notified
       */
      virtual void subscribe(
          std::shared_ptr<OrderingServiceNotification> subscriber) = 0;

      /**
       * Publishes proposal over network
       * @param proposal : proposal to be published
       */
      virtual void publishProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal,
          const std::vector<std::string> &peers) = 0;

      virtual ~OrderingServiceTransport() = default;
    };

  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_TRANSPORT_H
