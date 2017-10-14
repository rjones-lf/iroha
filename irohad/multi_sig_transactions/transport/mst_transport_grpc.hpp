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

#include <google/protobuf/empty.pb.h>
#include "logger/logger.hpp"
#include "model/converters/pb_transaction_factory.hpp"
#include "mst.grpc.pb.h"
#include "network/impl/async_grpc_client.hpp"
#include "network/mst_transport.hpp"

namespace iroha {
  namespace network {
    class MstTransportGrpc : public MstTransport,
                             public transport::MstTransportGrpc::Service,
                             private AsyncGrpcClient<google::protobuf::Empty> {
     public:
      MstTransportGrpc();

      /**
       * Server part of grpc SendState method call
       * @param context - server context with information about call
       * @param request - received new MstState object
       * @param response - buffer for response data, not used
       * @return grpc::Status (always OK)
       */
      grpc::Status SendState(
          ::grpc::ServerContext* context,
          const ::iroha::network::transport::MstState* request,
          ::google::protobuf::Empty* response) override;

      void subscribe(
          std::shared_ptr<MstTransportNotification> notification) override;

      void sendState(ConstRefPeer to, ConstRefState providing_state) override;

     private:
      std::weak_ptr<MstTransportNotification> subscriber_;
      model::converters::PbTransactionFactory factory_;
      logger::Logger log_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_MST_TRANSPORT_GRPC_HPP
