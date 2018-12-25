/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP
#define IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP

#include <google/protobuf/empty.pb.h>

#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_service_transport.hpp"
#include "ordering.grpc.pb.h"
#include "transaction.pb.h"

namespace iroha {
  namespace ordering {

    class OrderingServiceTransportGrpc
        : public iroha::network::OrderingServiceTransport,
          public proto::OrderingServiceTransportGrpc::Service {
     public:
      OrderingServiceTransportGrpc(
          std::shared_ptr<shared_model::interface::TransactionBatchFactory>
              transaction_batch_factory,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call);
      void subscribe(
          std::shared_ptr<iroha::network::OrderingServiceNotification>
              subscriber) override;

      void publishProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal,
          const std::vector<std::string> &peers) override;

      grpc::Status onBatch(::grpc::ServerContext *context,
                           const protocol::TxList *request,
                           ::google::protobuf::Empty *response) override;

      ~OrderingServiceTransportGrpc() = default;

     private:
      std::weak_ptr<iroha::network::OrderingServiceNotification> subscriber_;
      std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
          async_call_;
      std::shared_ptr<shared_model::interface::TransactionBatchFactory>
          batch_factory_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP
