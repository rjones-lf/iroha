/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ORDERING_SERVICE_IMPL_HPP
#define IROHA_ORDERING_SERVICE_IMPL_HPP

#include <memory>
#include <shared_mutex>

#include <tbb/concurrent_queue.h>
#include <rxcpp/rx.hpp>

#include "ametsuchi/os_persistent_state_factory.hpp"
#include "ametsuchi/peer_query_factory.hpp"
#include "interfaces/iroha_internal/proposal_factory.hpp"
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
      using TimeoutType = long;
      /**
       * Constructor
       * @param peer_query_factory interface for fetching peers from world state
       * view
       * @param max_size maximum size of proposal
       * @param proposal_timeout observable timeout for proposal creation
       * @param transport receive transactions and publish proposals
       * @param persistent_state factory to storage for auxiliary information
       * @param factory is used to generate proposals
       * @param is_async whether proposals are generated in a separate thread
       */
      OrderingServiceImpl(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          size_t max_size,
          rxcpp::observable<TimeoutType> proposal_timeout,
          std::shared_ptr<network::OrderingServiceTransport> transport,
          std::shared_ptr<ametsuchi::OsPersistentStateFactory> persistent_state,
          std::unique_ptr<shared_model::interface::ProposalFactory> factory,
          bool is_async = true);

      /**
       * Process transaction(s) received from network
       * Enqueues transactions and publishes corresponding event
       * @param batch, in which transactions are packed
       */
      void onBatch(shared_model::interface::TransactionBatch &&batch) override;

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
      enum class ProposalEvent { kBatchEvent, kTimerEvent };

      /**
       * Collect transactions from queue
       * Passes the generated proposal to publishProposal
       */
      void generateProposal() override;

      std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory_;

      tbb::concurrent_queue<
          std::unique_ptr<shared_model::interface::TransactionBatch>>
          queue_;

      /**
       * max number of txs in proposal
       */
      const size_t max_size_;

      /**
       * current number of transactions in a queue
       */
      std::atomic_ulong current_size_;

      std::shared_ptr<network::OrderingServiceTransport> transport_;

      /**
       * Factory to persistent storage for proposal counter.
       * In case of relaunch, ordering server will enumerate proposals
       * consecutively.
       */
      std::shared_ptr<ametsuchi::OsPersistentStateFactory> persistent_state_;

      /// Observable for transaction events from the network
      rxcpp::subjects::subject<ProposalEvent> transactions_;

      /// Internal event observable handle
      rxcpp::composite_subscription handle_;

      /**
       * Variables for concurrency
       */
      /// mutex for both batch and proposal generation
      std::shared_timed_mutex batch_prop_mutex_;
      /// mutex for events activating
      std::mutex event_mutex_;

      std::unique_ptr<shared_model::interface::ProposalFactory> factory_;

      /**
       * Proposal counter of expected proposal. Should be number of blocks in
       * the ledger + 1.
       */
      size_t proposal_height_;

      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_IMPL_HPP
