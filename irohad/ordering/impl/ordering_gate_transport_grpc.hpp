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

#ifndef ORDERING_GATE_TRANSPORT_GRPC_HPP
#define ORDERING_GATE_TRANSPORT_GRPC_HPP


#include <ordering.grpc.pb.h>
#include <model/converters/pb_transaction_factory.hpp>
#include "network/ordering_gate_transport.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace iroha {
  namespace ordering {

    class OrderingGateTransportGrpc : public iroha::network::OrderingGateTransport,
                                      public network::AsyncGrpcClient<google::protobuf::Empty>  {
    private:
      std::unique_ptr<proto::OrderingService::Stub> client_;
      model::converters::PbTransactionFactory factory_;

    public:

      explicit OrderingGateTransportGrpc(const std::string& server_address);

      OrderingGateTransportGrpc(const OrderingGateTransportGrpc& rhs) = delete;
      OrderingGateTransportGrpc& operator=(const OrderingGateTransportGrpc& rhs) = delete;

      OrderingGateTransportGrpc(OrderingGateTransportGrpc&& rhs) = delete;
      OrderingGateTransportGrpc& operator=(OrderingGateTransportGrpc&& rhs) = delete;

      void propagate(std::shared_ptr<const iroha::model::Transaction> transaction)  override;

      void subscribe(std::shared_ptr<iroha::network::OrderingGateNotification> subscriber) override;


      std::shared_ptr<model::Proposal> getProposal(const iroha::ordering::proto::Proposal *request) override;

      ~OrderingGateTransportGrpc() override = default;
    };
  }
}

#endif
