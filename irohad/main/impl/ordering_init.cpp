/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/ordering_init.hpp"
#include "ametsuchi/ordering_service_persistent_state.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace network {
    auto OrderingInit::createGate(
        std::shared_ptr<OrderingGateTransport> transport,
        std::shared_ptr<ametsuchi::BlockQuery> block_query) {
      return block_query->getTopBlock().match(
          [this, &transport](
              expected::Value<std::shared_ptr<shared_model::interface::Block>>
                  &block) -> std::shared_ptr<OrderingGate> {
            const auto &height = block.value->height();
            auto gate =
                std::make_shared<ordering::OrderingGateImpl>(transport, height);
            log_->info("Creating Ordering Gate with initial height {}", height);
            transport->subscribe(gate);
            return gate;
          },
          [](expected::Error<std::string> &error)
              -> std::shared_ptr<OrderingGate> {
            // TODO 12.06.18 Akvinikym: handle the exception IR-1415
            throw std::runtime_error("Ordering Gate creation failed! "
                                     + error.error);
          });
    }

    auto OrderingInit::createService(
        std::shared_ptr<ametsuchi::PeerQuery> wsv,
        size_t max_size,
        std::chrono::milliseconds delay_milliseconds,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state) {
      auto factory = std::make_unique<shared_model::proto::ProtoProposalFactory<
          shared_model::validation::DefaultProposalValidator>>();
      return std::make_shared<ordering::OrderingServiceImpl>(
          wsv,
          max_size,
          rxcpp::observable<>::interval(delay_milliseconds,
                                        rxcpp::observe_on_new_thread()),
          transport,
          persistent_state,
          std::move(factory));
    }

    std::shared_ptr<OrderingGate> OrderingInit::initOrderingGate(
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
