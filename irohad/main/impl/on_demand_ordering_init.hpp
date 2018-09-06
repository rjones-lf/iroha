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

#ifndef IROHA_ON_DEMAND_ORDERING_INIT_HPP
#define IROHA_ON_DEMAND_ORDERING_INIT_HPP

#include "ametsuchi/peer_query_factory.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "ordering.grpc.pb.h"
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
          std::chrono::milliseconds delay);

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
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
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

      ordering::transport::RejectRoundType current_reject_round_ = 1;
      std::vector<std::shared_ptr<shared_model::interface::Peer>>
          current_peers_;

      enum RoundType { kCurrentRound, kNextRound, kRoundAfterNext, kCount };

      template <RoundType Round>
      struct RoundTypeEnum {
        static constexpr auto round = Round;
      };

      std::array<std::vector<size_t>, kCount> permutations_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_INIT_HPP
