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

#ifndef IROHA_ORDERINGSERVICE_H
#define IROHA_ORDERINGSERVICE_H

#include "network/ordering_service_transport.hpp"

namespace iroha {
  namespace network {
    class OrderingService : public network::OrderingServiceNotification {
     public:
      /**
       * Collect transactions from queue
       * Passes the generated proposal to publishProposal
       */
      virtual void generateProposal() = 0;

      /**
       * Transform model proposal to transport object and send to peers
       * @param proposal - object for propagation
       */
      virtual void publishProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal) = 0;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_ORDERINGSERVICE_H
