/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TESTIROHAD_HPP
#define IROHA_TESTIROHAD_HPP

#include "cryptography/keypair.hpp"
#include "main/application.hpp"

namespace integration_framework {
  /**
   * Class for integration testing of Irohad.
   */
  class TestIrohad : public Irohad {
   public:
    TestIrohad(const std::string &block_store_dir,
               const std::string &pg_conn,
               const std::string &listen_ip,
               size_t torii_port,
               size_t internal_port,
               size_t max_proposal_size,
               std::chrono::milliseconds proposal_delay,
               std::chrono::milliseconds vote_delay,
               const shared_model::crypto::Keypair &keypair,
               const boost::optional<iroha::GossipPropagationStrategyParams>
                   &opt_mst_gossip_params = boost::none)
        : Irohad(block_store_dir,
                 pg_conn,
                 listen_ip,
                 torii_port,
                 internal_port,
                 max_proposal_size,
                 proposal_delay,
                 vote_delay,
                 keypair,
                 opt_mst_gossip_params) {}

    auto &getCommandService() {
      return command_service;
    }

    auto &getCommandServiceTransport() {
      return command_service_transport;
    }

    auto &getQueryService() {
      return query_service;
    }

    auto &getMstProcessor() {
      return mst_processor;
    }

    auto &getConsensusGate() {
      return consensus_gate;
    }

    auto &getPeerCommunicationService() {
      return pcs;
    }

    auto &getCryptoSigner() {
      return crypto_signer_;
    }

    auto getStatusBus() {
      return status_bus_;
    }

    void terminate() {
      if (internal_server) {
        internal_server->shutdown();
      } else {
        log_->warn("Tried to terminate without internal server");
      }
    }

    void terminate(const std::chrono::system_clock::time_point &deadline) {
      if (internal_server) {
        internal_server->shutdown(deadline);
      } else {
        log_->warn("Tried to terminate without internal server");
      }
    }
  };
}  // namespace integration_framework

#endif  // IROHA_TESTIROHAD_HPP
