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

#ifndef IROHA_APPLICATION_HPP
#define IROHA_APPLICATION_HPP

#include "ametsuchi/impl/peer_query_wsv.hpp"
#include "ametsuchi/impl/storage_impl.hpp"
#include "ametsuchi/ordering_service_persistent_state.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "cryptography/keypair.hpp"
#include "logger/logger.hpp"
#include "main/impl/block_loader_init.hpp"
#include "main/impl/consensus_init.hpp"
#include "main/impl/ordering_init.hpp"
#include "main/server_runner.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/impl/peer_communication_service_impl.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "simulator/block_creator.hpp"
#include "simulator/impl/simulator.hpp"
#include "synchronizer/impl/synchronizer_impl.hpp"
#include "synchronizer/synchronizer.hpp"
#include "torii/command_service.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii/query_service.hpp"
#include "validation/chain_validator.hpp"
#include "validation/impl/chain_validator_impl.hpp"
#include "validation/impl/stateful_validator_impl.hpp"
#include "validation/stateful_validator.hpp"

namespace iroha {
  namespace ametsuchi {
    class WsvRestorer;
  }
}  // namespace iroha

class Irohad {
 public:
  /**
   * Constructor that initializes common iroha pipeline
   * @param block_store_dir - folder where blocks will be stored
   * @param pg_conn - initialization string for postgre
   * @param torii_port - port for torii binding
   * @param internal_port - port for internal communication - ordering service,
   * consensus, and block loader
   * @param max_proposal_size - maximum transactions that possible appears in
   * one proposal
   * @param proposal_delay - maximum waiting time util emitting new proposal
   * @param vote_delay - waiting time before sending vote to next peer
   * @param load_delay - waiting time before loading committed block from next
   * peer
   * @param keypair - public and private keys for crypto signer
   */
  Irohad(const std::string &block_store_dir,
         const std::string &pg_conn,
         size_t torii_port,
         size_t internal_port,
         size_t max_proposal_size,
         std::chrono::milliseconds proposal_delay,
         std::chrono::milliseconds vote_delay,
         std::chrono::milliseconds load_delay,
         const shared_model::crypto::Keypair &keypair);

  /**
   * Initialization of whole objects in system
   */
  virtual void init();

  /**
   * Reset oredering service storage state to default
   */
  void resetOrderingService();

  /**
   * Restore World State View
   * @return true on success, false otherwise
   */
  bool restoreWsv();

  /**
   * Drop wsv and block store
   */
  virtual void dropStorage();

  /**
   * Run worker threads for start performing
   */
  virtual void run();

  virtual ~Irohad() = default;

 protected:
  // -----------------------| component initialization |------------------------

  virtual void initStorage();

  virtual void initPeerQuery();

  virtual void initCryptoProvider();

  virtual void initValidators();

  virtual void initOrderingGate();

  virtual void initSimulator();

  virtual void initBlockLoader();

  virtual void initConsensusGate();

  virtual void initSynchronizer();

  virtual void initPeerCommunicationService();

  virtual void initTransactionCommandService();

  virtual void initQueryService();

  /**
   * Initialize WSV restorer
   */
  virtual void initWsvRestorer();

  // constructor dependencies
  std::string block_store_dir_;
  std::string pg_conn_;
  size_t torii_port_;
  size_t internal_port_;
  size_t max_proposal_size_;
  std::chrono::milliseconds proposal_delay_;
  std::chrono::milliseconds vote_delay_;
  std::chrono::milliseconds load_delay_;

  // ------------------------| internal dependencies |-------------------------

  // crypto provider
  std::shared_ptr<shared_model::crypto::CryptoModelSigner<>> crypto_signer_;

  // validators
  std::shared_ptr<iroha::validation::StatefulValidator> stateful_validator;
  std::shared_ptr<iroha::validation::ChainValidator> chain_validator;

  // peer query
  std::shared_ptr<iroha::ametsuchi::PeerQuery> wsv;

  // WSV restorer
  std::shared_ptr<iroha::ametsuchi::WsvRestorer> wsv_restorer_;

  // ordering gate
  std::shared_ptr<iroha::network::OrderingGate> ordering_gate;

  // simulator
  std::shared_ptr<iroha::simulator::Simulator> simulator;

  // block loader
  std::shared_ptr<iroha::network::BlockLoader> block_loader;

  // consensus gate
  std::shared_ptr<iroha::network::ConsensusGate> consensus_gate;

  // synchronizer
  std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer;

  // pcs
  std::shared_ptr<iroha::network::PeerCommunicationService> pcs;

  // transaction service
  std::shared_ptr<torii::CommandService> command_service;

  // query service
  std::shared_ptr<torii::QueryService> query_service;

  // ordering service persistent state storage
  std::shared_ptr<iroha::ametsuchi::OrderingServicePersistentState>
      ordering_service_storage_;

  std::unique_ptr<ServerRunner> torii_server;
  std::unique_ptr<ServerRunner> internal_server;

  // initialization objects
  iroha::network::OrderingInit ordering_init;
  iroha::consensus::yac::YacInit yac_init;
  iroha::network::BlockLoaderInit loader_init;

  logger::Logger log_;

 public:
  std::shared_ptr<iroha::ametsuchi::Storage> storage;

  shared_model::crypto::Keypair keypair;
  grpc::ServerBuilder builder;
};

#endif  // IROHA_APPLICATION_HPP
