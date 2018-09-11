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

#ifndef IROHA_ORDERING_INIT_HPP
#define IROHA_ORDERING_INIT_HPP

#include "ametsuchi/block_query_factory.hpp"
#include "ametsuchi/peer_query_factory.hpp"
#include "ametsuchi/os_persistent_state_factory.hpp"
#include "logger/logger.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"
#include "ordering/impl/single_peer_ordering_service.hpp"

namespace iroha {

  namespace ametsuchi {
    class OrderingServicePersistentState;
  }

  namespace network {

    /**
     * Class aimed to effective initialization of OrderingGate component
     */
    class OrderingInit {
     private:
      /**
       * Init effective realisation of ordering gate (client of ordering
       * service)
       * @param transport - object which will be notified
       * about incoming proposals and send transactions
       * @param block_query_factory - block store factory to get last block
       * height
       * @return ordering gate
       */
      auto createGate(
          std::shared_ptr<OrderingGateTransport> transport,
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory);

      /**
       * Init ordering service
       * @param peer_query_factory - factory to get peer list
       * @param max_size - limitation of proposal size
       * @param delay_milliseconds - delay before emitting proposal
       * @param transport - ordering service transport
       * @param persistent_state - factory to access persistent state
       * @return ordering service
       */
      auto createService(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          size_t max_size,
          std::chrono::milliseconds delay_milliseconds,
          std::shared_ptr<network::OrderingServiceTransport> transport,
          std::shared_ptr<ametsuchi::OsPersistentStateFactory>
              persistent_state);

     public:
      /**
       * Initialization of ordering gate(client) and ordering service (service)
       * @param peer_query_factory - factory to get peer list
       * @param max_size - limitation of proposal size
       * @param delay_milliseconds - delay before emitting proposal
       * @param persistent_state - factory to access persistent state
       * @param block_query_factory - block store factory to get last block
       * height
       * @param async_call - async grpc client that is passed to transport
       * components
       * @return efficient implementation of OrderingGate
       */
      std::shared_ptr<iroha::network::OrderingGate> initOrderingGate(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          size_t max_size,
          std::chrono::milliseconds delay_milliseconds,
          std::shared_ptr<ametsuchi::OsPersistentStateFactory>
              persistent_state,
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call);

      std::shared_ptr<ordering::SinglePeerOrderingService> ordering_service;
      std::shared_ptr<iroha::network::OrderingGate> ordering_gate;
      std::shared_ptr<ordering::OrderingGateTransportGrpc>
          ordering_gate_transport;
      std::shared_ptr<ordering::OrderingServiceTransportGrpc>
          ordering_service_transport;

     protected:
      logger::Logger log_ = logger::log("OrderingInit");
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ORDERING_INIT_HPP
