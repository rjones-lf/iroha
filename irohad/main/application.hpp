/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_APPLICATION_HPP
#define IROHA_APPLICATION_HPP

#include "ametsuchi/impl/storage_impl.hpp"
#include "ametsuchi/tx_presence_cache.hpp"
#include "consensus/consensus_block_cache.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/iroha_internal/query_response_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "logger/logger.hpp"
#include "main/impl/block_loader_init.hpp"
#include "main/impl/consensus_init.hpp"
#include "main/impl/ordering_init.hpp"
#include "main/server_runner.hpp"
#include "multi_sig_transactions/gossip_propagation_strategy_params.hpp"
#include "multi_sig_transactions/mst_processor.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/impl/peer_communication_service_impl.hpp"
#include "network/mst_transport.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "pending_txs_storage/impl/pending_txs_storage_impl.hpp"
#include "simulator/block_creator.hpp"
#include "simulator/impl/simulator.hpp"
#include "synchronizer/impl/synchronizer_impl.hpp"
#include "synchronizer/synchronizer.hpp"
#include "torii/command_service.hpp"
#include "torii/impl/command_service_transport_grpc.hpp"
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
  using RunResult = iroha::expected::Result<void, std::string>;

  /**
   * Constructor that initializes common iroha pipeline
   * @param block_store_dir - folder where blocks will be stored
   * @param pg_conn - initialization string for postgre
   * @param listen_ip - ip address for opening ports (internal & torii)
   * @param torii_port - port for torii binding
   * @param internal_port - port for internal communication - ordering service,
   * consensus, and block loader
   * @param max_proposal_size - maximum transactions that possible appears in
   * one proposal
   * @param proposal_delay - maximum waiting time util emitting new proposal
   * @param vote_delay - waiting time before sending vote to next peer
   * @param keypair - public and private keys for crypto signer
   * @param opt_mst_gossip_params - parameters for Gossip MST propagation
   * (optional). If not provided, disables mst processing support
   *
   * TODO mboldyrev 03.11.2018 IR-1844 Refactor the constructor.
   */
  Irohad(const std::string &block_store_dir,
         const std::string &pg_conn,
         const std::string &listen_ip,
         size_t torii_port,
         size_t internal_port,
         size_t max_proposal_size,
         std::chrono::milliseconds proposal_delay,
         std::chrono::milliseconds vote_delay,
         const shared_model::crypto::Keypair &keypair,
         const boost::optional<iroha::GossipPropagationStrategyParams>
             &opt_mst_gossip_params = boost::none);

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
   * @return void value on success, error message otherwise
   */
  RunResult run();

  virtual ~Irohad();

 protected:
  // -----------------------| component initialization |------------------------

  virtual void initStorage();

  virtual void initCryptoProvider();

  virtual void initBatchParser();

  virtual void initValidators();

  virtual void initNetworkClient();

  virtual void initFactories();

  virtual void initOrderingGate();

  virtual void initSimulator();

  virtual void initConsensusCache();

  virtual void initBlockLoader();

  virtual void initPersistentCache();

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
  const std::string listen_ip_;
  size_t torii_port_;
  size_t internal_port_;
  size_t max_proposal_size_;
  std::chrono::milliseconds proposal_delay_;
  std::chrono::milliseconds vote_delay_;
  bool is_mst_supported_;
  boost::optional<iroha::GossipPropagationStrategyParams>
      opt_mst_gossip_params_;

  // ------------------------| internal dependencies |-------------------------

  // crypto provider
  std::shared_ptr<shared_model::crypto::CryptoModelSigner<>> crypto_signer_;

  // batch parser
  std::shared_ptr<shared_model::interface::TransactionBatchParser> batch_parser;

  // validators
  std::shared_ptr<iroha::validation::StatefulValidator> stateful_validator;
  std::shared_ptr<iroha::validation::ChainValidator> chain_validator;

  // WSV restorer
  std::shared_ptr<iroha::ametsuchi::WsvRestorer> wsv_restorer_;

  // async call
  std::shared_ptr<iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
      async_call_;

  // common objects factory
  std::shared_ptr<shared_model::interface::CommonObjectsFactory>
      common_objects_factory_;

  // transaction batch factory
  std::shared_ptr<shared_model::interface::TransactionBatchFactory>
      transaction_batch_factory_;

  // ordering gate
  std::shared_ptr<iroha::network::OrderingGate> ordering_gate;

  // simulator
  std::shared_ptr<iroha::simulator::Simulator> simulator;

  // block cache for consensus and block loader
  std::shared_ptr<iroha::consensus::ConsensusResultCache>
      consensus_result_cache_;

  // block loader
  std::shared_ptr<iroha::network::BlockLoader> block_loader;

  // persistent cache
  std::shared_ptr<iroha::ametsuchi::TxPresenceCache> persistent_cache;

  // consensus gate
  std::shared_ptr<iroha::network::ConsensusGate> consensus_gate;

  // synchronizer
  std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer;

  // pcs
  std::shared_ptr<iroha::network::PeerCommunicationService> pcs;

  // transaction factory
  std::shared_ptr<shared_model::interface::AbstractTransportFactory<
      shared_model::interface::Transaction,
      iroha::protocol::Transaction>>
      transaction_factory;

  // query factory
  std::shared_ptr<shared_model::interface::AbstractTransportFactory<
      shared_model::interface::Query,
      iroha::protocol::Query>>
      query_factory;

  // query response factory
  std::shared_ptr<shared_model::interface::QueryResponseFactory>
      query_response_factory_;

  // mst
  std::shared_ptr<iroha::MstProcessor> mst_processor;

  // pending transactions storage
  std::shared_ptr<iroha::PendingTransactionStorage> pending_txs_storage_;

  // status bus
  std::shared_ptr<iroha::torii::StatusBus> status_bus_;

  // transaction service
  std::shared_ptr<torii::CommandService> command_service;
  std::shared_ptr<torii::CommandServiceTransportGrpc> command_service_transport;

  // query service
  std::shared_ptr<torii::QueryService> query_service;

  std::unique_ptr<ServerRunner> torii_server;
  std::unique_ptr<ServerRunner> internal_server;

  // initialization objects
  iroha::network::OrderingInit ordering_init;
  iroha::consensus::yac::YacInit yac_init;
  iroha::network::BlockLoaderInit loader_init;

  std::shared_ptr<iroha::network::MstTransport> mst_transport;

  logger::Logger log_;

 public:
  std::shared_ptr<iroha::ametsuchi::Storage> storage;

  shared_model::crypto::Keypair keypair;
  grpc::ServerBuilder builder;
};

#endif  // IROHA_APPLICATION_HPP
