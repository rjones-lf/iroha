/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_TESTIROHAD_HPP
#define IROHA_TESTIROHAD_HPP

#include "main/application.hpp"

/**
 * Class for integration testing of Irohad.
 */
class TestIrohad : public Irohad {
 public:
  TestIrohad(const std::string &block_store_dir,
             const std::string &pg_conn,
             size_t torii_port,
             size_t internal_port,
             size_t max_proposal_size,
             std::chrono::milliseconds proposal_delay,
             std::chrono::milliseconds vote_delay,
             std::chrono::milliseconds load_delay,
             const iroha::keypair_t &keypair)
      : Irohad(block_store_dir,
               pg_conn,
               torii_port,
               internal_port,
               max_proposal_size,
               proposal_delay,
               vote_delay,
               load_delay,
               keypair) {}

  auto &getCommandService() {
    return command_service;
  }

  auto &getQueryService() {
    return query_service;
  }

  auto &getPeerCommunicationService() {
    return pcs;
  }

  auto &getCryptoProvider() {
    return crypto_verifier;
  }

  void run() override {
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort("0.0.0.0:" + std::to_string(internal_port_),
                             grpc::InsecureServerCredentials(),
                             &port);
    builder.RegisterService(ordering_init.ordering_gate_transport.get());
    builder.RegisterService(ordering_init.ordering_service_transport.get());
    builder.RegisterService(yac_init.consensus_network.get());
    builder.RegisterService(loader_init.service.get());
    internal_server = builder.BuildAndStart();
    BOOST_ASSERT_MSG(port != 0, "grpc port is 0");
    internal_thread = std::thread([this] { internal_server->Wait(); });
    log_->info("===> iroha initialized");
  }
};

#endif  // IROHA_TESTIROHAD_HPP
