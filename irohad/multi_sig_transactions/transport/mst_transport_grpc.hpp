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

#ifndef IROHA_MST_TRANSPORT_GRPC_HPP
#define IROHA_MST_TRANSPORT_GRPC_HPP

#include "mst.grpc.pb.h"
#include "network/mst_transport.hpp"

#include <google/protobuf/empty.pb.h>
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace iroha {
  namespace network {
    class MstTransportGrpc : public MstTransport,
                             public transport::MstTransportGrpc::Service {
     public:
      using TransportFactoryType =
          shared_model::interface::AbstractTransportFactory<
              shared_model::interface::Transaction,
              iroha::protocol::Transaction>;

      MstTransportGrpc(
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory,
          std::shared_ptr<TransportFactoryType> transaction_factory,
          std::shared_ptr<shared_model::interface::TransactionBatchParser>
              batch_parser);

      /**
       * Server part of grpc SendState method call
       * @param context - server context with information about call
       * @param request - received new MstState object
       * @param response - buffer for response data, not used
       * @return grpc::Status (always OK)
       */
      grpc::Status SendState(
          ::grpc::ServerContext *context,
          const ::iroha::network::transport::MstState *request,
          ::google::protobuf::Empty *response) override;

      void subscribe(
          std::shared_ptr<MstTransportNotification> notification) override;

      void sendState(const shared_model::interface::Peer &to,
                     ConstRefState providing_state) override;

     private:
      std::weak_ptr<MstTransportNotification> subscriber_;
      std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
          async_call_;
      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;
      std::shared_ptr<TransportFactoryType> transaction_factory_;
      std::shared_ptr<shared_model::interface::TransactionBatchParser>
          batch_parser_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_MST_TRANSPORT_GRPC_HPP
