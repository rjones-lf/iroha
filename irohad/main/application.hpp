/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_APPLICATION_HPP
#define IROHA_APPLICATION_HPP

#include "consensus/consensus_block_cache.hpp"
#include "cryptography/crypto_provider/abstract_crypto_model_signer.hpp"
#include "interfaces/queries/query.hpp"
#include "logger/logger.hpp"
#include "main/impl/block_loader_init.hpp"
#include "main/impl/consensus_init.hpp"
#include "main/impl/on_demand_ordering_init.hpp"
#include "multi_sig_transactions/gossip_propagation_strategy_params.hpp"

namespace iroha {
  class PendingTransactionStorage;
  class MstProcessor;
  namespace ametsuchi {
    class WsvRestorer;
    class TxPresenceCache;
    class Storage;
  }  // namespace ametsuchi
  namespace network {
    class BlockLoader;
    class ConsensusGate;
    class PeerCommunicationService;
    class MstTransport;
    class OrderingGate;
  }  // namespace network
  namespace simulator {
    class Simulator;
  }
  namespace synchronizer {
    class Synchronizer;
  }
  namespace torii {
    class QueryProcessor;
    class StatusBus;
    class CommandService;
    class CommandServiceTransportGrpc;
    class QueryService;
  }  // namespace torii
  namespace validation {
    class ChainValidator;
    class StatefulValidator;
  }  // namespace validation
}  // namespace iroha

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
  namespace interface {
    class CommonObjectsFactory;
    class QueryResponseFactory;
    class TransactionBatchFactory;
  }  // namespace interface
}  // namespace shared_model

class ServerRunner;

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
   * @param mst_expiration_time - maximum time until until MST transaction is
   * not considered as expired (in minutes)
   * @param keypair - public and private keys for crypto signer
   * @param max_rounds_delay - maximum delay between consecutive rounds without
   * transactions
   * @param stale_stream_max_rounds - maximum number of rounds between
   * consecutive status emissions
   * @param opt_mst_gossip_params - parameters for Gossip MST propagation
   * (optional). If not provided, disables mst processing support
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
         std::chrono::minutes mst_expiration_time,
         const shared_model::crypto::Keypair &keypair,
         std::chrono::milliseconds max_rounds_delay,
         size_t stale_stream_max_rounds,
         const boost::optional<iroha::GossipPropagationStrategyParams>
             &opt_mst_gossip_params = boost::none);

  /**
   * Initialization of whole objects in system
   */
  virtual void init();

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

  virtual void initPersistentCache();

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
  const std::string listen_ip_;
  size_t torii_port_;
  size_t internal_port_;
  size_t max_proposal_size_;
  std::chrono::milliseconds proposal_delay_;
  std::chrono::milliseconds vote_delay_;
  bool is_mst_supported_;
  std::chrono::minutes mst_expiration_time_;
  std::chrono::milliseconds max_rounds_delay_;
  size_t stale_stream_max_rounds_;
  boost::optional<iroha::GossipPropagationStrategyParams>
      opt_mst_gossip_params_;

  // ------------------------| internal dependencies |-------------------------
 public:
  shared_model::crypto::Keypair keypair;
  std::shared_ptr<iroha::ametsuchi::Storage> storage;

 protected:
  logger::Logger log_;

  // initialization objects
  iroha::network::OnDemandOrderingInit ordering_init;
  iroha::consensus::yac::YacInit yac_init;
  iroha::network::BlockLoaderInit loader_init;

  // common objects factory
  std::shared_ptr<shared_model::interface::CommonObjectsFactory>
      common_objects_factory_;

  // WSV restorer
  std::shared_ptr<iroha::ametsuchi::WsvRestorer> wsv_restorer_;

  // crypto provider
  std::shared_ptr<shared_model::crypto::AbstractCryptoModelSigner<
      shared_model::interface::Block>>
      crypto_signer_;

  // batch parser
  std::shared_ptr<shared_model::interface::TransactionBatchParser> batch_parser;

  // validators
  std::shared_ptr<iroha::validation::StatefulValidator> stateful_validator;
  std::shared_ptr<iroha::validation::ChainValidator> chain_validator;

  // async call
  std::shared_ptr<iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
      async_call_;

  // transaction batch factory
  std::shared_ptr<shared_model::interface::TransactionBatchFactory>
      transaction_batch_factory_;

  // transaction factory
  std::shared_ptr<shared_model::interface::AbstractTransportFactory<
      shared_model::interface::Transaction,
      iroha::protocol::Transaction>>
      transaction_factory;

  // query response factory
  std::shared_ptr<shared_model::interface::QueryResponseFactory>
      query_response_factory_;

  // query factory
  std::shared_ptr<shared_model::interface::AbstractTransportFactory<
      shared_model::interface::Query,
      iroha::protocol::Query>>
      query_factory;

  // persistent cache
  std::shared_ptr<iroha::ametsuchi::TxPresenceCache> persistent_cache;

  // proposal factory
  std::shared_ptr<shared_model::interface::AbstractTransportFactory<
      shared_model::interface::Proposal,
      iroha::protocol::Proposal>>
      proposal_factory;

  // ordering gate
  std::shared_ptr<iroha::network::OrderingGate> ordering_gate;

  // simulator
  std::shared_ptr<iroha::simulator::Simulator> simulator;

  // block cache for consensus and block loader
  std::shared_ptr<iroha::consensus::ConsensusResultCache>
      consensus_result_cache_;

  // block loader
  std::shared_ptr<iroha::network::BlockLoader> block_loader;

  // consensus gate
  std::shared_ptr<iroha::network::ConsensusGate> consensus_gate;
  rxcpp::subjects::subject<iroha::consensus::GateObject> consensus_gate_objects;
  rxcpp::composite_subscription consensus_gate_events_subscription;

  // synchronizer
  std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer;

  // pcs
  std::shared_ptr<iroha::network::PeerCommunicationService> pcs;

  // status bus
  std::shared_ptr<iroha::torii::StatusBus> status_bus_;

  // mst
  std::shared_ptr<iroha::network::MstTransport> mst_transport;
  std::shared_ptr<iroha::MstProcessor> mst_processor;

  // pending transactions storage
  std::shared_ptr<iroha::PendingTransactionStorage> pending_txs_storage_;

  // transaction service
  std::shared_ptr<iroha::torii::CommandService> command_service;
  std::shared_ptr<iroha::torii::CommandServiceTransportGrpc>
      command_service_transport;

  // query service
  std::shared_ptr<iroha::torii::QueryService> query_service;

  std::unique_ptr<ServerRunner> torii_server;
  std::unique_ptr<ServerRunner> internal_server;
};

#endif  // IROHA_APPLICATION_HPP
