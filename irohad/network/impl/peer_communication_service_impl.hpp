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

#ifndef IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP
#define IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP

#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "synchronizer/synchronizer.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace network {
    class PeerCommunicationServiceImpl : public PeerCommunicationService {
     public:
      PeerCommunicationServiceImpl(
          std::shared_ptr<OrderingGate> ordering_gate,
          std::shared_ptr<synchronizer::Synchronizer> synchronizer);

      void propagate_transaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() const override;

      rxcpp::observable<Commit> on_commit() const override;

     private:
      std::shared_ptr<OrderingGate> ordering_gate_;
      std::shared_ptr<synchronizer::Synchronizer> synchronizer_;
      logger::Logger log_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP
