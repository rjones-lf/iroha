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
#ifndef IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_H
#define IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_H


#include <ordering.pb.h>

#include <model/converters/pb_transaction_factory.hpp>
#include <network/ordering_service_transport.hpp>
#include "network/ordering_gate_transport.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace iroha {
  namespace ordering {

    class OrderingServiceTransportGrpc : public iroha::network::OrderingServiceTransport,
                                         public network::AsyncGrpcClient<google::protobuf::Empty> {
      void publishProposal(std::shared_ptr<proto::Proposal> proposal,
                           const std::unique_ptr<proto::OrderingGate::Stub>& peer) override;


    };

  }
}
#endif //IROHA_ORDERING_SERVICE_TRANSPORT_GRPC_H
