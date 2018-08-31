/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP
#define IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP

#include "ordering/on_demand_os_transport.hpp"

#include "network/impl/async_grpc_client.hpp"
#include "ordering.grpc.pb.h"

namespace iroha {
  namespace ordering {
    namespace transport {

      /**
       * gRPC client for on demand ordering service
       */
      class OnDemandOsClientGrpc : public OdOsNotification {
       public:
        using TimepointType = std::chrono::system_clock::time_point;
        using TimeoutType = std::chrono::milliseconds;

        /**
         * Constructor is left public because testing required passing a mock
         * stub interface
         */
        OnDemandOsClientGrpc(
            std::unique_ptr<proto::OnDemandOrdering::StubInterface> stub,
            std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
                async_call,
            std::function<TimepointType()> time_provider,
            std::chrono::milliseconds proposal_request_timeout);

        void onTransactions(transport::RoundType round,
                            CollectionType transactions) override;

        boost::optional<ProposalType> onRequestProposal(
            transport::RoundType round) override;

       private:
        logger::Logger log_;
        std::unique_ptr<proto::OnDemandOrdering::StubInterface> stub_;
        std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
            async_call_;
        std::function<TimepointType()> time_provider_;
        std::chrono::milliseconds proposal_request_timeout_;
      };

      class OnDemandOsClientGrpcFactory : public OdOsNotificationFactory {
       public:
        OnDemandOsClientGrpcFactory(
            std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
                async_call,
            std::function<OnDemandOsClientGrpc::TimepointType()> time_provider,
            OnDemandOsClientGrpc::TimeoutType proposal_request_timeout);

        /**
         * Create connection with insecure gRPC channel defined by
         * network::createClient method
         * @see network/impl/grpc_channel_builder.hpp
         * This factory method can be used in production code
         */
        std::unique_ptr<OdOsNotification> create(
            const shared_model::interface::Peer &to) override;

       private:
        std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
            async_call_;
        std::function<OnDemandOsClientGrpc::TimepointType()> time_provider_;
        std::chrono::milliseconds proposal_request_timeout_;
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP
