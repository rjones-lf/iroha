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

#include "main/impl/ordering_init.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

namespace iroha {
  namespace network {
    auto OrderingInit::createGate(std::string network_address) {
      auto transport = std::make_shared<iroha::ordering::OrderingGateTransportGrpc>(network_address);
      auto gate_impl = std::make_shared<ordering::OrderingGateImpl>(transport);
      return gate_impl;
    }

    auto OrderingInit::createService(std::shared_ptr<ametsuchi::PeerQuery> wsv,
                                     size_t max_size,
                                     size_t delay_milliseconds,
                                     std::shared_ptr<uvw::Loop> loop) {

      auto transport = std::make_shared<ordering::OrderingServiceTransportGrpc>();
      return std::make_shared<ordering::OrderingServiceImpl>(wsv,
                                                             max_size,
                                                             delay_milliseconds,
                                                             transport,
                                                             loop);
    }

    std::shared_ptr<ordering::OrderingGateImpl> OrderingInit::initOrderingGate(
        std::shared_ptr<ametsuchi::PeerQuery> wsv,
        std::shared_ptr<uvw::Loop> loop,
        size_t max_size,
        size_t delay_milliseconds) {
      ordering_service =
          createService(wsv, max_size, delay_milliseconds, loop);
      ordering_gate = createGate(wsv->getLedgerPeers().value().front().address);
      return ordering_gate;
    }
  }  // namespace network
}  // namespace iroha
