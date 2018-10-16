/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TEST_MSTNET_IROHAD_HPP
#define IROHA_TEST_MSTNET_IROHAD_HPP

#include <gmock/gmock.h>

#include "cryptography/keypair.hpp"
#include "main/application.hpp"
#include "multi_sig_transactions/gossip_propagation_strategy.hpp"
#include "multi_sig_transactions/mst_processor_impl.hpp"
#include "multi_sig_transactions/mst_propagation_strategy_stub.hpp"
#include "multi_sig_transactions/mst_time_provider_impl.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

namespace integration_framework {

  struct MockTransportGrpc : public iroha::network::MstTransportGrpc {
    using MstTransportGrpc::MstTransportGrpc;
    MOCK_METHOD2(sendState,
                 void(const shared_model::interface::Peer &,
                      iroha::ConstRefState));
  };
  /**
   * Class for integration testing of Irohad.
   */
  class TestMstnetIrohad : public Irohad {
   public:
    TestMstnetIrohad(
        const std::string &block_store_dir,
        const std::string &pg_conn,
        size_t torii_port,
        size_t internal_port,
        size_t max_proposal_size,
        std::chrono::milliseconds proposal_delay,
        std::chrono::milliseconds vote_delay,
        const shared_model::crypto::Keypair &keypair,
        bool is_mst_supported,
        std::function<void(std::shared_ptr<MockTransportGrpc>)> mocker)
        : Irohad(block_store_dir,
                 pg_conn,
                 torii_port,
                 internal_port,
                 max_proposal_size,
                 proposal_delay,
                 vote_delay,
                 keypair,
                 is_mst_supported),
          mocker(mocker) {}

    auto &getCommandServiceTransport() {
      return command_service_transport;
    }

    void initMstProcessor() override {
      auto mock =
          std::make_shared<MockTransportGrpc>(async_call_,
                                              common_objects_factory_,
                                              transaction_factory,
                                              batch_parser,
                                              transaction_batch_factory_);
      mocker(mock);
      mst_transport = mock;
      auto mst_completer = std::make_shared<iroha::DefaultCompleter>();
      auto mst_storage =
          std::make_shared<iroha::MstStorageStateImpl>(mst_completer);
      std::shared_ptr<iroha::PropagationStrategy> mst_propagation;
      if (is_mst_supported_) {
        mst_propagation = std::make_shared<iroha::GossipPropagationStrategy>(
            storage,
            std::chrono::seconds(5) /*emitting period*/,
            2 /*amount per once*/);
      } else {
        mst_propagation = std::make_shared<iroha::PropagationStrategyStub>();
      }

      auto mst_time = std::make_shared<iroha::MstTimeProviderImpl>();
      auto fair_mst_processor = std::make_shared<iroha::FairMstProcessor>(
          mst_transport, mst_storage, mst_propagation, mst_time);
      mst_processor = fair_mst_processor;
      mst_transport->subscribe(fair_mst_processor);
      log_->info("[Init] => MST processor");
    }

    void run() override {
      internal_server = std::make_unique<ServerRunner>(
          "127.0.0.1:" + std::to_string(internal_port_));
      internal_server->append(ordering_init.ordering_gate_transport)
          .append(ordering_init.ordering_service_transport)
          .append(yac_init.consensus_network)
          .append(loader_init.service)
          .run()
          .match([](iroha::expected::Value<int>) {},
                 [](iroha::expected::Error<std::string> e) {
                   BOOST_ASSERT_MSG(false, e.error.c_str());
                 });
      log_->info("===> iroha initialized");
    }

    void terminate() {
      if (internal_server) {
        internal_server->shutdown();
      }
    }

   private:
    std::function<void(std::shared_ptr<MockTransportGrpc>)> mocker;
  };
}  // namespace integration_framework

#endif  // IROHA_TEST_MSTNET_IROHAD_HPP
