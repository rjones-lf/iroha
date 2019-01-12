/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_INIT_HPP
#define IROHA_ON_DEMAND_ORDERING_INIT_HPP

#include <random>

#include "ametsuchi/peer_query_factory.hpp"
#include "ametsuchi/tx_presence_cache.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "ordering.grpc.pb.h"
#include "ordering/impl/on_demand_os_server_grpc.hpp"
#include "ordering/impl/ordering_gate_cache/ordering_gate_cache.hpp"
#include "ordering/on_demand_ordering_service.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace network {

    /**
     * Encapsulates initialization logic for on-demand ordering gate and service
     */
    class OnDemandOrderingInit {
     private:
      /**
       * Creates notification factory for individual connections to peers with
       * gRPC backend. \see initOrderingGate for parameters
       */
      auto createNotificationFactory(
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::chrono::milliseconds delay);

      /**
       * Creates connection manager which redirects requests to appropriate
       * ordering services in the current round. \see initOrderingGate for
       * parameters
       */
      auto createConnectionManager(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::chrono::milliseconds delay,
          std::vector<shared_model::interface::types::HashType> initial_hashes);

      /**
       * Creates on-demand ordering gate. \see initOrderingGate for parameters
       * TODO andrei 31.10.18 IR-1825 Refactor ordering gate observable
       */
      auto createGate(
          std::shared_ptr<ordering::OnDemandOrderingService> ordering_service,
          std::shared_ptr<ordering::transport::OdOsNotification> network_client,
          std::shared_ptr<ordering::cache::OrderingGateCache> cache,
          std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
          consensus::Round initial_round,
          std::function<std::chrono::seconds(
              const synchronizer::SynchronizationEvent &)> delay_func);

      /**
       * Creates on-demand ordering service. \see initOrderingGate for
       * parameters
       */
      auto createService(
          size_t max_size,
          std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache);

     public:
      ~OnDemandOrderingInit();

      /**
       * Initializes on-demand ordering gate and ordering sevice components
       *
       * @param max_size maximum number of transaction in a proposal
       * @param delay timeout for ordering service response on proposal request
       * @param initial_hashes seeds for peer list permutations for first k
       * rounds they are required since hash of block i defines round i + k
       * @param peer_query_factory factory for getLedgerPeers query required by
       * connection manager
       * @param transaction_factory transport factory for transactions required
       * by ordering service network endpoint
       * @param batch_parser transaction batch parser required by ordering
       * service network endpoint
       * @param transaction_batch_factory transport factory for transaction
       * batch candidates produced by parser
       * @param async_call asynchronous gRPC client required for sending batches
       * requests to ordering service and processing responses
       * @param proposal_factory factory required by ordering service to produce
       * proposals
       * @param initial_round initial value for current round used in
       * OnDemandOrderingGate
       * @return initialized ordering gate
       */
      std::shared_ptr<network::OrderingGate> initOrderingGate(
          size_t max_size,
          std::chrono::milliseconds delay,
          std::vector<shared_model::interface::types::HashType> initial_hashes,
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          std::shared_ptr<
              ordering::transport::OnDemandOsServerGrpc::TransportFactoryType>
              transaction_factory,
          std::shared_ptr<shared_model::interface::TransactionBatchParser>
              batch_parser,
          std::shared_ptr<shared_model::interface::TransactionBatchFactory>
              transaction_batch_factory,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
          consensus::Round initial_round,
          std::function<std::chrono::seconds(
              const synchronizer::SynchronizationEvent &)> delay_func);

      /// gRPC service for ordering service
      std::shared_ptr<ordering::proto::OnDemandOrdering::Service> service;

      /// commit notifier from peer communication service
      rxcpp::subjects::subject<decltype(
          std::declval<PeerCommunicationService>().on_commit())::value_type>
          notifier;

     private:
      logger::Logger log_ = logger::log("OnDemandOrderingInit");

      std::vector<std::shared_ptr<shared_model::interface::Peer>>
          current_peers_;

      /// indexes to permutations for corresponding rounds
      enum RoundType { kCurrentRound, kNextRound, kRoundAfterNext, kCount };

      template <RoundType V>
      using RoundTypeConstant = std::integral_constant<RoundType, V>;

      /// permutations for peers lists
      std::array<std::vector<size_t>, kCount> permutations_;

      /// random generator for peer list permutations
      // TODO andrei 08.11.2018 IR-1850 Refactor default_random_engine usages
      // with platform-independent class
      std::default_random_engine gen_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_INIT_HPP
