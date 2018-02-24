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

#include "ametsuchi/peer_query.hpp"
#include "logger/logger.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_impl.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

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
       * @param network_address - address of ordering service
       */
      auto createGate(std::shared_ptr<OrderingGateTransport>);

      /**
       * Init ordering service
       * @param peers - endpoints of peers for connection
       * @param max_size - limitation of proposal size
       * @param delay_milliseconds - delay before emitting proposal
       * @param loop - handler of async events
       */
      auto createService(
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          size_t max_size,
          std::chrono::milliseconds delay_milliseconds,
          std::shared_ptr<network::OrderingServiceTransport> transport,
          std::shared_ptr<ametsuchi::OrderingServicePersistentState>
              persistent_state);

     public:
      /**
       * Initialization of ordering gate(client) and ordering service (service)
       * @param peers - endpoints of peers for connection
       * @param loop - handler of async events
       * @param max_size - limitation of proposal size
       * @param delay_milliseconds - delay before emitting proposal
       * @return effective realisation of OrderingGate
       */
      std::shared_ptr<ordering::OrderingGateImpl> initOrderingGate(
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          size_t max_size,
          std::chrono::milliseconds delay_milliseconds,
          std::shared_ptr<ametsuchi::OrderingServicePersistentState>
              persistent_state);

      std::shared_ptr<ordering::OrderingServiceImpl> ordering_service;
      std::shared_ptr<ordering::OrderingGateImpl> ordering_gate;
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
