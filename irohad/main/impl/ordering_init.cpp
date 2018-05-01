/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/ordering_init.hpp"
#include "ametsuchi/ordering_service_persistent_state.hpp"
#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  namespace network {
    auto OrderingInit::createGate(
        std::shared_ptr<OrderingGateTransport> transport,
        std::shared_ptr<ametsuchi::BlockQuery> block_query) {
      auto height = block_query->getTopBlocks(1).as_blocking().last()->height();
      auto gate =
          std::make_shared<ordering::OrderingGateImpl>(transport, height);
      log_->info("Creating Ordering Gate with initial height {}", height);
      transport->subscribe(gate);
      return gate;
    }

    auto OrderingInit::createService(
        std::shared_ptr<ametsuchi::PeerQuery> wsv,
        size_t max_size,
        std::chrono::milliseconds delay_milliseconds,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state) {
      return std::make_shared<ordering::OrderingServiceImpl>(
          wsv,
          max_size,
          rxcpp::observable<>::interval(delay_milliseconds,
                                        rxcpp::observe_on_new_thread()),
          transport,
          persistent_state);
    }

    std::shared_ptr<ordering::OrderingGateImpl> OrderingInit::initOrderingGate(
        std::shared_ptr<ametsuchi::PeerQuery> wsv,
        size_t max_size,
        std::chrono::milliseconds delay_milliseconds,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state,
        std::shared_ptr<ametsuchi::BlockQuery> block_query) {
      auto ledger_peers = wsv->getLedgerPeers();
      if (not ledger_peers or ledger_peers.value().empty()) {
        log_->error(
            "Ledger don't have peers. Do you set correct genesis block?");
      }
      auto network_address = ledger_peers->front()->address();
      log_->info("Ordering gate is at {}", network_address);
      ordering_gate_transport =
          std::make_shared<iroha::ordering::OrderingGateTransportGrpc>(
              network_address);

      ordering_service_transport =
          std::make_shared<ordering::OrderingServiceTransportGrpc>();
      ordering_service = createService(wsv,
                                       max_size,
                                       delay_milliseconds,
                                       ordering_service_transport,
                                       persistent_state);
      ordering_service_transport->subscribe(ordering_service);
      ordering_gate = createGate(ordering_gate_transport, block_query);
      return ordering_gate;
    }
  }  // namespace network
}  // namespace iroha
