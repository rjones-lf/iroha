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
#ifndef IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP
#define IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP

#include <google/protobuf/empty.pb.h>

#include "block.pb.h"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_service_transport.hpp"
#include "ordering.grpc.pb.h"

namespace iroha {
  namespace ordering {

    class OrderingServiceTransportGrpc
        : public iroha::network::OrderingServiceTransport,
          public proto::OrderingServiceTransportGrpc::Service,
          network::AsyncGrpcClient<google::protobuf::Empty> {
     public:
      OrderingServiceTransportGrpc();
      void subscribe(
          std::shared_ptr<iroha::network::OrderingServiceNotification>
              subscriber) override;

      void publishProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal,
          const std::vector<std::string> &peers) override;

      grpc::Status onTransaction(::grpc::ServerContext *context,
                                 const protocol::Transaction *request,
                                 ::google::protobuf::Empty *response) override;

      ~OrderingServiceTransportGrpc() = default;

     private:
      std::weak_ptr<iroha::network::OrderingServiceNotification> subscriber_;
      logger::Logger log_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_HPP
