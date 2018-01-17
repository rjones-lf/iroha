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

#include "main/application.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::simulator;
using namespace iroha::validation;
using namespace iroha::network;
using namespace iroha::model;
using namespace iroha::synchronizer;
using namespace iroha::torii;
using namespace iroha::model::converters;
using namespace iroha::consensus::yac;

/*
 * Configuring iroha daemon
 */
Irohad::Irohad(const std::string &block_store_dir,
               const std::string &redis_host,
               size_t redis_port,
               const std::string &pg_conn,
               size_t torii_port,
               size_t internal_port,
               size_t max_proposal_size,
               std::chrono::milliseconds proposal_delay,
               std::chrono::milliseconds vote_delay,
               std::chrono::milliseconds load_delay,
               const keypair_t &keypair)
    : block_store_dir_(block_store_dir),
      redis_host_(redis_host),
      redis_port_(redis_port),
      pg_conn_(pg_conn),
      torii_port_(torii_port),
      internal_port_(internal_port),
      max_proposal_size_(max_proposal_size),
      proposal_delay_(proposal_delay),
      vote_delay_(vote_delay),
      load_delay_(load_delay),
      keypair(keypair) {
  log_ = logger::log("IROHAD");
  log_->info("created");
  // Initializing storage at this point in order to read genesis block before
  // initialization of iroha deamon
  initStorage();
}

Irohad::~Irohad() {
  // Shutting down grpc server
  if (internal_server) {
    internal_server->Shutdown();
  }
  // Shutting down torii server
  if (torii_server) {
    torii_server->shutdown();
  }
  // Waiting until grpc server thread die
  if (internal_thread.joinable()) {
    internal_thread.join();
  }
  // Waiting until torii server thread die
  if (server_thread.joinable()) {
    server_thread.join();
  }
}

/*
 * Initializing iroha daemon
 */
void Irohad::init() {
  initProtoFactories();
  initPeerQuery();
  initCryptoProvider();
  initValidators();
  initOrderingGate();
  initSimulator();
  initBlockLoader();
  initConsensusGate();
  initSynchronizer();
  initPeerCommunicationService();

  // Torii
  initTransactionCommandService();
  initQueryService();
}

/*
 * Dropping iroha daemon storage
 */
void Irohad::dropStorage() {
  storage->dropStorage();
}

/*
 * Initializing iroha daemon storage
 */
void Irohad::initStorage() {
  storage =
      StorageImpl::create(block_store_dir_, redis_host_, redis_port_, pg_conn_);

  log_->info("[Init] => storage", logger::logBool(storage));
}

/*
 * Creating transaction, query and query response factories
 */
void Irohad::initProtoFactories() {
  pb_tx_factory = std::make_shared<PbTransactionFactory>();
  pb_query_factory = std::make_shared<PbQueryFactory>();
  pb_query_response_factory = std::make_shared<PbQueryResponseFactory>();

  log_->info("[Init] => converters");
}

/*
 * Initializing peer query interface
 */
void Irohad::initPeerQuery() {
  wsv = std::make_shared<ametsuchi::PeerQueryWsv>(storage->getWsvQuery());

  log_->info("[Init] => peer query");
}

/*
 * Initializing crypto provider
 */
void Irohad::initCryptoProvider() {
  crypto_verifier = std::make_shared<ModelCryptoProviderImpl>(keypair);

  log_->info("[Init] => crypto provider");
}

/*
 * Initializing validators
 */
void Irohad::initValidators() {
  stateless_validator =
      std::make_shared<StatelessValidatorImpl>(crypto_verifier);
  stateful_validator = std::make_shared<StatefulValidatorImpl>();
  chain_validator = std::make_shared<ChainValidatorImpl>();

  log_->info("[Init] => validators");
}

/*
 * Initializing ordering gate
 */
void Irohad::initOrderingGate() {
  ordering_gate =
      ordering_init.initOrderingGate(wsv, max_proposal_size_, proposal_delay_);
  log_->info("[Init] => init ordering gate - [{}]",
             logger::logBool(ordering_gate));
}

/*
 * Initializing iroha proposal creator and block creator
 */
void Irohad::initSimulator() {
  simulator = std::make_shared<Simulator>(ordering_gate,
                                          stateful_validator,
                                          storage,
                                          storage->getBlockQuery(),
                                          crypto_verifier);

  log_->info("[Init] => init simulator");
}

/*
 * Initializing block loader
 */
void Irohad::initBlockLoader() {
  block_loader = loader_init.initBlockLoader(
      wsv, storage->getBlockQuery(), crypto_verifier);

  log_->info("[Init] => block loader");
}

/*
 * Initializing consensus gate
 */
void Irohad::initConsensusGate() {
  consensus_gate = yac_init.initConsensusGate(
      wsv, simulator, block_loader, keypair, vote_delay_, load_delay_);

  log_->info("[Init] => consensus gate");
}

/*
 * Initializing synchronizer
 */
void Irohad::initSynchronizer() {
  synchronizer = std::make_shared<SynchronizerImpl>(
      consensus_gate, chain_validator, storage, block_loader);

  log_->info("[Init] => synchronizer");
}

/*
 * Initializing peer communication service
 */
void Irohad::initPeerCommunicationService() {
  pcs = std::make_shared<PeerCommunicationServiceImpl>(ordering_gate,
                                                       synchronizer);

  pcs->on_proposal().subscribe(
      [this](auto) { log_->info("~~~~~~~~~| PROPOSAL ^_^ |~~~~~~~~~ "); });

  pcs->on_commit().subscribe(
      [this](auto) { log_->info("~~~~~~~~~| COMMIT =^._.^= |~~~~~~~~~ "); });

  log_->info("[Init] => pcs");
}

/*
 * Initializing transaction command service
 */
void Irohad::initTransactionCommandService() {
  auto tx_processor =
      std::make_shared<TransactionProcessorImpl>(pcs, stateless_validator);

  command_service = std::make_unique<::torii::CommandService>(
      pb_tx_factory, tx_processor, storage);

  log_->info("[Init] => command service");
}

/*
 * Initializing query command service
 */
void Irohad::initQueryService() {
  auto query_processing_factory = std::make_unique<QueryProcessingFactory>(
      storage->getWsvQuery(), storage->getBlockQuery());

  auto query_processor = std::make_shared<QueryProcessorImpl>(
      std::move(query_processing_factory), stateless_validator);

  query_service = std::make_unique<::torii::QueryService>(
      pb_query_factory, pb_query_response_factory, query_processor);

  log_->info("[Init] => query service");
}
/*
 * Run iroha deamon
 */
void Irohad::run() {
  // Initializing torii server
  torii_server =
      std::make_unique<ServerRunner>("0.0.0.0:" + std::to_string(torii_port_));

  // Initializing grpc server
  grpc::ServerBuilder builder;
  int port = 0;
  builder.AddListeningPort("0.0.0.0:" + std::to_string(internal_port_),
                           grpc::InsecureServerCredentials(),
                           &port);
  builder.RegisterService(ordering_init.ordering_gate_transport.get());
  builder.RegisterService(ordering_init.ordering_service_transport.get());
  builder.RegisterService(yac_init.consensus_network.get());
  builder.RegisterService(loader_init.service.get());
  // Run grpc server
  internal_server = builder.BuildAndStart();
  // Run torii server
  server_thread = std::thread([this] {
    torii_server->run(std::move(command_service), std::move(query_service));
  });
  log_->info("===> iroha initialized");
  // Wait until servers shutdown
  torii_server->waitForServersReady();
  internal_server->Wait();
}
