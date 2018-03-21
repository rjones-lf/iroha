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

#ifndef IROHA_ORDERING_GATE_IMPL_HPP
#define IROHA_ORDERING_GATE_IMPL_HPP

#include "network/ordering_gate.hpp"

#include <atomic>
#include <tbb/concurrent_queue.h>

#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate_transport.hpp"

namespace shared_model {
  namespace interaface {
    class Transaction;
    class Proposal;
  }  // namespace interaface
}  // namespace shared_model

namespace iroha {
  namespace ordering {

    /**
     * OrderingGate implementation with gRPC asynchronous client
     * Interacts with given OrderingService
     * by propagating transactions and receiving proposals
     * @param server_address OrderingService address
     */
    class OrderingGateImpl : public network::OrderingGate,
                             public network::OrderingGateNotification {
     public:
      explicit OrderingGateImpl(
          std::shared_ptr<iroha::network::OrderingGateTransport> transport);

      void propagateTransaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() override;

      void setPcs(const iroha::network::PeerCommunicationService &pcs) override;

      void onProposal(
          std::shared_ptr<shared_model::interface::Proposal> proposal) override;

      ~OrderingGateImpl() override;

     private:
      /**
       * Try to push proposal for next consensus round
       */
      void tryNextRound();

      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::Proposal>>
          proposals_;
      std::shared_ptr<iroha::network::OrderingGateTransport> transport_;

      /// invariant: true if proposal can be pushed to subscribers
      std::atomic_bool unlock_next_{true};

      /// queue with all proposals received from ordering service
      tbb::concurrent_queue<std::shared_ptr<shared_model::interface::Proposal>>
          proposal_queue_;

      /// subscription of pcs::on_commit
      rxcpp::composite_subscription pcs_subscriber_;

      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_GATE_IMPL_HPP
