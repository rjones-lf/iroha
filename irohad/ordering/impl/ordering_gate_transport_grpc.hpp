/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IROHA_ORDERING_GATE_TRANSPORT_GRPC_H
#define IROHA_ORDERING_GATE_TRANSPORT_GRPC_H

#include <google/protobuf/empty.pb.h>

#include "backend/protobuf/proto_proposal_factory.hpp"
#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate_transport.hpp"
#include "ordering.grpc.pb.h"
#include "validators/default_validator.hpp"

namespace iroha {
  namespace ordering {
    class OrderingGateTransportGrpc
        : public iroha::network::OrderingGateTransport,
          public proto::OrderingGateTransportGrpc::Service {
     public:
      OrderingGateTransportGrpc(
          const std::string &server_address,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call);

      grpc::Status onProposal(::grpc::ServerContext *context,
                              const protocol::Proposal *request,
                              ::google::protobuf::Empty *response) override;

      void propagateBatch(
          std::shared_ptr<shared_model::interface::TransactionBatch> batch)
          override;

      void subscribe(std::shared_ptr<iroha::network::OrderingGateNotification>
                         subscriber) override;

     private:
      std::weak_ptr<iroha::network::OrderingGateNotification> subscriber_;
      std::unique_ptr<proto::OrderingServiceTransportGrpc::StubInterface>
          client_;
      std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
          async_call_;
      std::unique_ptr<shared_model::proto::ProtoProposalFactory<
          shared_model::validation::DefaultProposalValidator>>
          factory_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_GATE_TRANSPORT_GRPC_H
