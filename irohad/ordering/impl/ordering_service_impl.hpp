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

#include <tbb/concurrent_queue.h>
#include <memory>
#include <unordered_map>

#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_service.hpp"
#include "network/ordering_service_transport.hpp"

#include "ametsuchi/peer_query.hpp"
#include "ordering.grpc.pb.h"

#include <rxcpp/rx.hpp>
#include "model/converters/pb_transaction_factory.hpp"
#include "model/proposal.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "ordering.grpc.pb.h"

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
    class OrderingServiceImpl : public network::OrderingService {
     public:
      OrderingServiceImpl(
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          size_t max_size,
          size_t delay_milliseconds,
          std::shared_ptr<network::OrderingServiceTransport> transport);

      /**
       * Process transaction received from network
       * Enqueues transaction and publishes corresponding event
       * @param transaction
       */
      void onTransaction(const model::Transaction &transaction) override;

      ~OrderingServiceImpl() override;

     protected:
      /**
       * Transform model proposal to transport object and send to peers
       * @param proposal - object for propagation
       */
      void publishProposal(model::Proposal &&proposal) override;

     private:
      /**
       * Collect transactions from queue
       * Passes the generated proposal to publishProposal
       */
      void generateProposal() override;

      /**
       * Method update peers for sending proposal
       */

      /**
       * Update the timer to be called after delay_milliseconds_
       */
      void updateTimer();

      rxcpp::observable<long> timer;
      rxcpp::composite_subscription handle;
      std::shared_ptr<ametsuchi::PeerQuery> wsv_;

      tbb::concurrent_queue<model::Transaction> queue_;

      /**
       * max number of txs in proposal
       */
      const size_t max_size_;

      /**
       *  wait for specified time if queue is empty
       */
      const size_t delay_milliseconds_;
      std::shared_ptr<network::OrderingServiceTransport> transport_;
      size_t expected_proposal_height;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_IMPL_HPP
