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

#include <mutex>

#include <tbb/concurrent_priority_queue.h>

#include "interfaces/common_objects/types.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate_transport.hpp"

namespace shared_model {
  namespace interface {
    class Transaction;
    class Proposal;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ordering {

    /**
     * Compare proposals by height
     */
    struct ProposalComparator {
      bool operator()(
          const std::shared_ptr<shared_model::interface::Proposal> &lhs,
          const std::shared_ptr<shared_model::interface::Proposal> &rhs) const;
    };

    /**
     * OrderingGate implementation with gRPC asynchronous client
     * Interacts with given OrderingService
     * by propagating transactions and receiving proposals
     * @param server_address OrderingService address
     */
    class OrderingGateImpl : public network::OrderingGate,
                             public network::OrderingGateNotification {
     public:
      /**
       * @param transport - network communication layer
       * @param initial_height - height of the last block stored on this peer
       * @param run_async - whether proposals should be handled
       * asynchronously (on separate thread). Default is true.
       */
      OrderingGateImpl(
          std::shared_ptr<iroha::network::OrderingGateTransport> transport,
          shared_model::interface::types::HeightType initial_height,
          bool run_async = true);

      void propagateTransaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) const override;

      void propagateBatch(const shared_model::interface::TransactionBatch
                              &batch) const override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() override;

      void setPcs(const iroha::network::PeerCommunicationService &pcs) override;

      void onProposal(
          std::shared_ptr<shared_model::interface::Proposal> proposal) override;

      ~OrderingGateImpl() override;

     private:
      /**
       * Try to push proposal for next consensus round
       * @param - last_block_height - what is the last block stored on this
       * peer, or for which commit was received. If block is newer than
       * currently stored proposals, proposals are discarded. If it is older,
       * newer proposals are propagated in order
       */
      void tryNextRound(
          shared_model::interface::types::HeightType last_block_height);

      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::Proposal>>
          proposals_;

      /**
       * Notification subject which is used only for notification purposes
       * without semantic for emitted values
       */
      rxcpp::subjects::subject<shared_model::interface::types::HeightType>
          net_proposals_;
      std::shared_ptr<iroha::network::OrderingGateTransport> transport_;

      std::mutex proposal_mutex_;

      /// queue with all proposals received from ordering service
      tbb::concurrent_priority_queue<
          std::shared_ptr<shared_model::interface::Proposal>,
          ProposalComparator>
          proposal_queue_;

      /// last commited block height
      shared_model::interface::types::HeightType last_block_height_;

      /// subscription of pcs::on_commit
      rxcpp::composite_subscription pcs_subscriber_;

      logger::Logger log_;

      bool run_async_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_GATE_IMPL_HPP
