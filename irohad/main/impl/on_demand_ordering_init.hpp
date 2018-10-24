/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_INIT_HPP
#define IROHA_ON_DEMAND_ORDERING_INIT_HPP

#include "ametsuchi/peer_query_factory.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "ordering.grpc.pb.h"
#include "ordering/impl/on_demand_os_server_grpc.hpp"
#include "ordering/on_demand_ordering_service.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace network {

    class OnDemandOrderingInit {
     private:
      auto createNotificationFactory(
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::chrono::milliseconds delay);

      auto createConnectionManager(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::chrono::milliseconds delay,
          std::vector<shared_model::interface::types::HashType> hashes);

      auto createGate(
          std::shared_ptr<ordering::OnDemandOrderingService> ordering_service,
          std::shared_ptr<ordering::transport::OdOsNotification> network_client,
          std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              factory);

      auto createService(size_t max_size);

     public:
      std::shared_ptr<network::OrderingGate> initOrderingGate(
          size_t max_size,
          std::chrono::milliseconds delay,
          std::vector<shared_model::interface::types::HashType> hashes,
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
          std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              factory);

      std::shared_ptr<ordering::proto::OnDemandOrdering::Service> service;

      rxcpp::subjects::subject<decltype(
          std::declval<PeerCommunicationService>().on_commit())::value_type>
          notifier;

     private:
      logger::Logger log_ = logger::log("OnDemandOrderingInit");

      consensus::RejectRoundType current_reject_round_ = 1;
      std::vector<std::shared_ptr<shared_model::interface::Peer>>
          current_peers_;

      /// indexes to permutations for corresponding rounds
      enum RoundType { kCurrentRound, kNextRound, kRoundAfterNext, kCount };

      template <RoundType V>
      using RoundTypeConstant = std::integral_constant<RoundType, V>;

      /// permutations for peers lists
      std::array<std::vector<size_t>, kCount> permutations_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_INIT_HPP
