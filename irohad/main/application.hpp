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

#include "ametsuchi/impl/storage_impl.hpp"
#include "consensus/consensus_block_cache.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "cryptography/keypair.hpp"
#include "logger/logger.hpp"
#include "main/impl/block_loader_init.hpp"
#include "main/impl/consensus_init.hpp"
#include "main/impl/ordering_init.hpp"
#include "main/server_runner.hpp"
#include "mst.grpc.pb.h"
#include "multi_sig_transactions/mst_processor.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/impl/peer_communication_service_impl.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "pending_txs_storage/impl/pending_txs_storage_impl.hpp"
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
   * @param keypair - public and private keys for crypto signer
   * @param is_mst_supported - enable or disable mst processing support
   */
  Irohad(const std::string &block_store_dir,
         const std::string &pg_conn,
         size_t torii_port,
         size_t internal_port,
         size_t max_proposal_size,
         std::chrono::milliseconds proposal_delay,
         std::chrono::milliseconds vote_delay,
         const shared_model::crypto::Keypair &keypair,
         bool is_mst_supported);

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

  virtual void initCryptoProvider();

  virtual void initValidators();

  virtual void initNetworkClient();

  virtual void initOrderingGate();

  virtual void initSimulator();

  virtual void initConsensusCache();

  virtual void initBlockLoader();

  virtual void initConsensusGate();

  virtual void initSynchronizer();

  virtual void initPeerCommunicationService();

  virtual void initStatusBus();

  virtual void initMstProcessor();

  virtual void initPendingTxsStorage();

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
  bool is_mst_supported_;

  // ------------------------| internal dependencies |-------------------------

  // crypto provider
  std::shared_ptr<shared_model::crypto::CryptoModelSigner<>> crypto_signer_;

  // validators
  std::shared_ptr<iroha::validation::StatefulValidator> stateful_validator;
  std::shared_ptr<iroha::validation::ChainValidator> chain_validator;

  // WSV restorer
  std::shared_ptr<iroha::ametsuchi::WsvRestorer> wsv_restorer_;

  // async call
  std::shared_ptr<iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
      async_call_;

  // ordering gate
  std::shared_ptr<iroha::network::OrderingGate> ordering_gate;

  // simulator
  std::shared_ptr<iroha::simulator::Simulator> simulator;

  // block cache for consensus and block loader
  std::shared_ptr<iroha::consensus::ConsensusResultCache> consensus_result_cache_;

  // block loader
  std::shared_ptr<iroha::network::BlockLoader> block_loader;

  // consensus gate
  std::shared_ptr<iroha::network::ConsensusGate> consensus_gate;

  // synchronizer
  std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer;

  // pcs
  std::shared_ptr<iroha::network::PeerCommunicationService> pcs;

  // mst
  std::shared_ptr<iroha::MstProcessor> mst_processor;

  // pending transactions storage
  std::shared_ptr<iroha::PendingTransactionStorage> pending_txs_storage_;

  // status bus
  std::shared_ptr<iroha::torii::StatusBus> status_bus_;

  // transaction service
  std::shared_ptr<torii::CommandService> command_service;

  // query service
  std::shared_ptr<torii::QueryService> query_service;

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
