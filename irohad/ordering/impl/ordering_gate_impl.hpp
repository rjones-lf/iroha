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

#ifndef IROHA_ORDERING_GATE_IMPL_HPP
#define IROHA_ORDERING_GATE_IMPL_HPP

#include "network/ordering_gate_transport.hpp"
#include "model/converters/pb_transaction_factory.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "network/ordering_gate.hpp"
#include "ordering.grpc.pb.h"

#include "logger/logger.hpp"

namespace iroha {
  namespace ordering {

    /**
     * OrderingGate implementation with gRPC asynchronous client
     * Interacts with given OrderingService
     * by propagating transactions and receiving proposals
     * @param server_address OrderingService address
     */
    class OrderingGateImpl : public network::OrderingGate,
                             public proto::OrderingGate::Service,
                             public network::OrderingGateNotification,
                             network::AsyncGrpcClient<google::protobuf::Empty> {
     public:

      explicit OrderingGateImpl(std::shared_ptr<network::OrderingGateTransport>);

      void propagate_transaction(
          std::shared_ptr<const model::Transaction> transaction) override;

      rxcpp::observable<model::Proposal> on_proposal() override;

      grpc::Status SendProposal(::grpc::ServerContext *context,
                                const proto::Proposal *request,
                                ::google::protobuf::Empty *response) override;

      std::shared_ptr<network::OrderingGateTransport> transport_;

    private:
      /**
       * Process proposal received from network
       * Publishes proposal to on_proposal subscribers
       * @param proposal
       */
      void handleProposal(std::shared_ptr<model::Proposal> proposal) override;

      rxcpp::subjects::subject<model::Proposal> proposals_;
      model::converters::PbTransactionFactory factory_;
      std::unique_ptr<proto::OrderingService::Stub> client_;
      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_GATE_IMPL_HPP
