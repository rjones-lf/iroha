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

#ifndef IROHA_ORDERING_SERVICE_IMPL_HPP
#define IROHA_ORDERING_SERVICE_IMPL_HPP

#include <memory>
#include <tbb/concurrent_queue.h>
#include <unordered_map>
#include <uvw.hpp>
#include "model/converters/pb_transaction_factory.hpp"
#include "model/proposal.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "ordering.grpc.pb.h"
#include "ametsuchi/peer_query.hpp"
#include "network/ordering_service_transport.hpp"

namespace iroha {
  namespace ordering {

    /**
     * OrderingService implementation with gRPC synchronous server
     * Allows receiving transactions concurrently from multiple peers by using
     * concurrent queue
     * Sends proposal by given timer interval and proposal size
     * @param delay_milliseconds timer delay
     * @param max_size proposal size
     */
    class OrderingServiceImpl
        : public proto::OrderingService::Service,
          public uvw::Emitter<OrderingServiceImpl> {
     public:
      OrderingServiceImpl(
          std::shared_ptr<ametsuchi::PeerQuery> wsv, size_t max_size,
          size_t delay_milliseconds,
          std::shared_ptr<network::OrderingServiceTransport> transport,
          std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault());

      grpc::Status SendTransaction(
          ::grpc::ServerContext *context, const protocol::Transaction *request,
          ::google::protobuf::Empty *response) override;
      ~OrderingServiceImpl() override;

     private:
      /**
       * Process transaction received from network
       * Enqueues transaction and publishes corresponding event
       * @param transaction
       */
      void handleTransaction(model::Transaction &&transaction);

      /**
       * Collect transactions from queue
       * Passes the generated proposal to publishProposal
       */
      void generateProposal();

      /**
       * Transform model proposal to transport object and send to peers
       * @param proposal - object for propagation
       */
      void publishProposal(model::Proposal &&proposal);

      /**
       * Method update peers for sending proposal
       */
      void preparePeersForProposalRound();

      std::shared_ptr<uvw::Loop> loop_;
      std::shared_ptr<uvw::TimerHandle> timer_;
      std::shared_ptr<ametsuchi::PeerQuery> wsv_;
      std::shared_ptr<network::OrderingServiceTransport> transport_;

      model::converters::PbTransactionFactory factory_;

      std::unordered_map<std::string,
                         std::unique_ptr<proto::OrderingGate::Stub>> peers_;

      tbb::concurrent_queue<model::Transaction> queue_;

      /**
       * max number of txs in proposal
       */
      const size_t max_size_;

      /**
       *  wait for specified time if queue is empty
       */
      const size_t delay_milliseconds_;
      size_t proposal_height;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_IMPL_HPP
