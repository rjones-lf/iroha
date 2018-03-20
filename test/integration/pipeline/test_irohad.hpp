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

#include "cryptography/keypair.hpp"
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
             const shared_model::crypto::Keypair &keypair)
      : Irohad(block_store_dir,
               pg_conn,
               torii_port,
               internal_port,
               max_proposal_size,
               proposal_delay,
               vote_delay,
               load_delay,
               *std::unique_ptr<iroha::keypair_t>(keypair.makeOldModel())) {}

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
    internal_server = std::make_unique<ServerRunner>(
        "0.0.0.0:" + std::to_string(internal_port_));
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
};

#endif  // IROHA_TESTIROHAD_HPP
