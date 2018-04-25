/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
#include <mutex>
#include <rxcpp/rx.hpp>

#include "logger/logger.hpp"
#include "network/ordering_service.hpp"
#include "ordering.grpc.pb.h"

namespace iroha {

  namespace ametsuchi {
    class OrderingServicePersistentState;
    class PeerQuery;
  }  // namespace ametsuchi

  namespace ordering {

    /**
     * OrderingService implementation with gRPC synchronous server
     * Allows receiving transactions concurrently from multiple peers by using
     * concurrent queue
     * Sends proposal by given timer interval and proposal size
     */
    class OrderingServiceImpl : public network::OrderingService {
     public:
      /**
       * Constructor
       * @param wsv interface for fetching peers from world state view
       * @param max_size maximum size of proposal
       * @param delay_milliseconds timeout for proposal generation
       * @param transport receive transactions and publish proposals
       * @param persistent_state storage for auxiliary information
       * @param is_async whether proposals are generated in a separate thread
       */
      OrderingServiceImpl(
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          size_t max_size,
          size_t delay_milliseconds,
          std::shared_ptr<network::OrderingServiceTransport> transport,
          std::shared_ptr<ametsuchi::OrderingServicePersistentState>
              persistent_state,
          bool is_async = true);

      /**
       * Process transaction received from network
       * Enqueues transaction and publishes corresponding event
       * @param transaction
       */
      void onTransaction(std::shared_ptr<shared_model::interface::Transaction>
                             transaction) override;

      ~OrderingServiceImpl() override;

     protected:
      /**
       * Transform model proposal to transport object and send to peers
       * @param proposal - object for propagation
       */
      void publishProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal) override;

     private:
      /**
       * Events for queue check strategy
       */
      enum class ProposalEvent { kTransactionEvent, kTimerEvent };

      /**
       * Collect transactions from queue
       * Passes the generated proposal to publishProposal
       */
      void generateProposal() override;

      std::shared_ptr<ametsuchi::PeerQuery> wsv_;

      tbb::concurrent_queue<
          std::shared_ptr<shared_model::interface::Transaction>>
          queue_;

      /**
       * max number of txs in proposal
       */
      const size_t max_size_;

      /**
       *  wait for specified time if queue is empty
       */
      const size_t delay_milliseconds_;

      std::shared_ptr<network::OrderingServiceTransport> transport_;

      /**
       * Persistent storage for proposal counter.
       * In case of relaunch, ordering server will enumerate proposals
       * consecutively.
       */
      std::shared_ptr<ametsuchi::OrderingServicePersistentState>
          persistent_state_;

      /**
       * Proposal counter of expected proposal. Should be number of blocks in
       * the ledger + 1.
       */
      size_t proposal_height_;

      /// Observable for transaction events from the network
      rxcpp::subjects::subject<ProposalEvent> transactions_;

      /// Internal event observable handle
      rxcpp::composite_subscription handle_;

      /**
       * Mutex for incoming transactions
       */
      std::mutex mutex_;

      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_IMPL_HPP
