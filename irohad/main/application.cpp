/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/application.hpp"

#include "ametsuchi/impl/postgres_ordering_service_persistent_state.hpp"
#include "ametsuchi/impl/wsv_restorer_impl.hpp"
#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_block_json_converter.hpp"
#include "backend/protobuf/proto_permission_to_string.hpp"
#include "backend/protobuf/proto_proposal_factory.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "backend/protobuf/proto_transport_factory.hpp"
#include "backend/protobuf/proto_tx_status_factory.hpp"
#include "common/bind.hpp"
#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"
#include "interfaces/permission_to_string.hpp"
#include "multi_sig_transactions/gossip_propagation_strategy.hpp"
#include "multi_sig_transactions/mst_processor_impl.hpp"
#include "multi_sig_transactions/mst_propagation_strategy_stub.hpp"
#include "multi_sig_transactions/mst_time_provider_impl.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"
#include "multi_sig_transactions/transport/mst_transport_stub.hpp"
#include "torii/impl/command_service_impl.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "validators/field_validator.hpp"
#include "validators/protobuf/proto_block_validator.hpp"
#include "validators/protobuf/proto_query_validator.hpp"
#include "validators/protobuf/proto_transaction_validator.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::simulator;
using namespace iroha::validation;
using namespace iroha::network;
using namespace iroha::synchronizer;
using namespace iroha::torii;
using namespace iroha::consensus::yac;

using namespace std::chrono_literals;

/**
 * Configuring iroha daemon
 */
Irohad::Irohad(const std::string &block_store_dir,
               const std::string &pg_conn,
               const std::string &listen_ip,
               size_t torii_port,
               size_t internal_port,
               size_t max_proposal_size,
               std::chrono::milliseconds proposal_delay,
               std::chrono::milliseconds vote_delay,
               const shared_model::crypto::Keypair &keypair,
               const boost::optional<GossipPropagationStrategyParams>
                   &opt_mst_gossip_params)
    : block_store_dir_(block_store_dir),
      pg_conn_(pg_conn),
      listen_ip_(listen_ip),
      torii_port_(torii_port),
      internal_port_(internal_port),
      max_proposal_size_(max_proposal_size),
      proposal_delay_(proposal_delay),
      vote_delay_(vote_delay),
      is_mst_supported_(opt_mst_gossip_params),
      opt_mst_gossip_params_(opt_mst_gossip_params),
      keypair(keypair) {
  log_ = logger::log("IROHAD");
  log_->info("created");
  // Initializing storage at this point in order to insert genesis block before
  // initialization of iroha daemon
  initStorage();
}

/**
 * Initializing iroha daemon
 */
void Irohad::init() {
  // Recover VSW from the existing ledger to be sure it is consistent
  initWsvRestorer();
  restoreWsv();

  initCryptoProvider();
  initBatchParser();
  initValidators();
  initNetworkClient();
  initFactories();
  initOrderingGate();
  initSimulator();
  initConsensusCache();
  initBlockLoader();
  initConsensusGate();
  initSynchronizer();
  initPeerCommunicationService();
  initStatusBus();
  initMstProcessor();
  initPendingTxsStorage();

  // Torii
  initTransactionCommandService();
  initQueryService();
}

/**
 * Dropping iroha daemon storage
 */
void Irohad::dropStorage() {
  storage->reset();
  storage->createOsPersistentState() |
      [](const auto &state) { state->resetState(); };
}

/**
 * Initializing iroha daemon storage
 */
void Irohad::initStorage() {
  common_objects_factory_ =
      std::make_shared<shared_model::proto::ProtoCommonObjectsFactory<
          shared_model::validation::FieldValidator>>();
  auto perm_converter =
      std::make_shared<shared_model::proto::ProtoPermissionToString>();
  auto block_converter =
      std::make_shared<shared_model::proto::ProtoBlockJsonConverter>();
  auto storageResult = StorageImpl::create(block_store_dir_,
                                           pg_conn_,
                                           common_objects_factory_,
                                           std::move(block_converter),
                                           perm_converter);
  storageResult.match(
      [&](expected::Value<std::shared_ptr<ametsuchi::StorageImpl>> &_storage) {
        storage = _storage.value;
      },
      [&](expected::Error<std::string> &error) { log_->error(error.error); });

  log_->info("[Init] => storage", logger::logBool(storage));
}

void Irohad::resetOrderingService() {
  if (not(storage->createOsPersistentState() |
          [](const auto &state) { return state->resetState(); }))
    log_->error("cannot reset ordering service storage");
}

bool Irohad::restoreWsv() {
  return wsv_restorer_->restoreWsv(*storage).match(
      [](iroha::expected::Value<void> v) { return true; },
      [&](iroha::expected::Error<std::string> &error) {
        log_->error(error.error);
        return false;
      });
}

/**
 * Initializing crypto provider
 */
void Irohad::initCryptoProvider() {
  crypto_signer_ =
      std::make_shared<shared_model::crypto::CryptoModelSigner<>>(keypair);

  log_->info("[Init] => crypto provider");
}

void Irohad::initBatchParser() {
  batch_parser =
      std::make_shared<shared_model::interface::TransactionBatchParserImpl>();

  log_->info("[Init] => transaction batch parser");
}

/**
 * Initializing validators
 */
void Irohad::initValidators() {
  auto factory = std::make_unique<shared_model::proto::ProtoProposalFactory<
      shared_model::validation::DefaultProposalValidator>>();
  stateful_validator =
      std::make_shared<StatefulValidatorImpl>(std::move(factory), batch_parser);
  chain_validator = std::make_shared<ChainValidatorImpl>(
      std::make_shared<consensus::yac::SupermajorityCheckerImpl>());

  log_->info("[Init] => validators");
}

/**
 * Initializing network client
 */
void Irohad::initNetworkClient() {
  async_call_ =
      std::make_shared<network::AsyncGrpcClient<google::protobuf::Empty>>();
}

void Irohad::initFactories() {
  // transaction factories
  transaction_batch_factory_ =
      std::make_shared<shared_model::interface::TransactionBatchFactoryImpl>();

  std::unique_ptr<shared_model::validation::AbstractValidator<
      shared_model::interface::Transaction>>
      transaction_validator =
          std::make_unique<shared_model::validation::
                               DefaultOptionalSignedTransactionValidator>();
  std::unique_ptr<
      shared_model::validation::AbstractValidator<iroha::protocol::Transaction>>
      proto_transaction_validator = std::make_unique<
          shared_model::validation::ProtoTransactionValidator>();
  transaction_factory =
      std::make_shared<shared_model::proto::ProtoTransportFactory<
          shared_model::interface::Transaction,
          shared_model::proto::Transaction>>(
          std::move(transaction_validator),
          std::move(proto_transaction_validator));

  // query factories
  query_response_factory_ =
      std::make_shared<shared_model::proto::ProtoQueryResponseFactory>();

  std::unique_ptr<shared_model::validation::AbstractValidator<
      shared_model::interface::Query>>
      query_validator = std::make_unique<
          shared_model::validation::DefaultSignedQueryValidator>();
  std::unique_ptr<
      shared_model::validation::AbstractValidator<iroha::protocol::Query>>
      proto_query_validator =
          std::make_unique<shared_model::validation::ProtoQueryValidator>();
  query_factory = std::make_shared<
      shared_model::proto::ProtoTransportFactory<shared_model::interface::Query,
                                                 shared_model::proto::Query>>(
      std::move(query_validator), std::move(proto_query_validator));

  log_->info("[Init] => factories");
}

/**
 * Initializing ordering gate
 */
void Irohad::initOrderingGate() {
  ordering_gate = ordering_init.initOrderingGate(storage,
                                                 max_proposal_size_,
                                                 proposal_delay_,
                                                 storage,
                                                 storage,
                                                 transaction_batch_factory_,
                                                 async_call_);
  log_->info("[Init] => init ordering gate - [{}]",
             logger::logBool(ordering_gate));
}

/**
 * Initializing iroha verified proposal creator and block creator
 */
void Irohad::initSimulator() {
  auto block_factory = std::make_unique<shared_model::proto::ProtoBlockFactory>(
      //  Block factory in simulator uses UnsignedBlockValidator because it is
      //  not required to check signatures of block here, as they will be
      //  checked when supermajority of peers will sign the block. It is also
      //  not required to validate signatures of transactions here because they
      //  are validated in the ordering gate, where they are received from the
      //  ordering service.
      std::make_unique<
          shared_model::validation::DefaultUnsignedBlockValidator>(),
      std::make_unique<shared_model::validation::ProtoBlockValidator>());
  simulator = std::make_shared<Simulator>(ordering_gate,
                                          stateful_validator,
                                          storage,
                                          storage,
                                          crypto_signer_,
                                          std::move(block_factory));

  log_->info("[Init] => init simulator");
}

/**
 * Initializing consensus block cache
 */
void Irohad::initConsensusCache() {
  consensus_result_cache_ = std::make_shared<consensus::ConsensusResultCache>();

  log_->info("[Init] => init consensus block cache");
}

/**
 * Initializing block loader
 */
void Irohad::initBlockLoader() {
  block_loader =
      loader_init.initBlockLoader(storage, storage, consensus_result_cache_);

  log_->info("[Init] => block loader");
}

/**
 * Initializing consensus gate
 */
void Irohad::initConsensusGate() {
  consensus_gate = yac_init.initConsensusGate(storage,
                                              simulator,
                                              block_loader,
                                              keypair,
                                              consensus_result_cache_,
                                              vote_delay_,
                                              async_call_,
                                              common_objects_factory_);

  log_->info("[Init] => consensus gate");
}

/**
 * Initializing synchronizer
 */
void Irohad::initSynchronizer() {
  synchronizer = std::make_shared<SynchronizerImpl>(
      consensus_gate, chain_validator, storage, block_loader);

  log_->info("[Init] => synchronizer");
}

/**
 * Initializing peer communication service
 */
void Irohad::initPeerCommunicationService() {
  pcs = std::make_shared<PeerCommunicationServiceImpl>(
      ordering_gate, synchronizer, simulator);

  pcs->on_proposal().subscribe(
      [this](auto) { log_->info("~~~~~~~~~| PROPOSAL ^_^ |~~~~~~~~~ "); });

  pcs->on_commit().subscribe(
      [this](auto) { log_->info("~~~~~~~~~| COMMIT =^._.^= |~~~~~~~~~ "); });

  // complete initialization of ordering gate
  ordering_gate->setPcs(*pcs);

  log_->info("[Init] => pcs");
}

void Irohad::initStatusBus() {
  status_bus_ = std::make_shared<StatusBusImpl>();
  log_->info("[Init] => Tx status bus");
}

void Irohad::initMstProcessor() {
  auto mst_completer = std::make_shared<DefaultCompleter>();
  auto mst_storage = std::make_shared<MstStorageStateImpl>(mst_completer);
  std::shared_ptr<iroha::PropagationStrategy> mst_propagation;
  if (is_mst_supported_) {
    mst_transport = std::make_shared<iroha::network::MstTransportGrpc>(
        async_call_,
        transaction_factory,
        batch_parser,
        transaction_batch_factory_,
        keypair.publicKey());
    mst_propagation = std::make_shared<GossipPropagationStrategy>(
        storage, rxcpp::observe_on_new_thread(), *opt_mst_gossip_params_);
  } else {
    mst_propagation = std::make_shared<iroha::PropagationStrategyStub>();
    mst_transport = std::make_shared<iroha::network::MstTransportStub>();
  }

  auto mst_time = std::make_shared<MstTimeProviderImpl>();
  auto fair_mst_processor = std::make_shared<FairMstProcessor>(
      mst_transport, mst_storage, mst_propagation, mst_time);
  mst_processor = fair_mst_processor;
  mst_transport->subscribe(fair_mst_processor);
  log_->info("[Init] => MST processor");
}

void Irohad::initPendingTxsStorage() {
  pending_txs_storage_ = std::make_shared<PendingTransactionStorageImpl>(
      mst_processor->onStateUpdate(),
      mst_processor->onPreparedBatches(),
      mst_processor->onExpiredBatches());
  log_->info("[Init] => pending transactions storage");
}

/**
 * Initializing transaction command service
 */
void Irohad::initTransactionCommandService() {
  auto status_factory =
      std::make_shared<shared_model::proto::ProtoTxStatusFactory>();
  auto tx_processor = std::make_shared<TransactionProcessorImpl>(
      pcs, mst_processor, status_bus_, status_factory);
  command_service = std::make_shared<::torii::CommandServiceImpl>(
      tx_processor, storage, status_bus_, status_factory);
  command_service_transport =
      std::make_shared<::torii::CommandServiceTransportGrpc>(
          command_service,
          status_bus_,
          std::chrono::seconds(1),
          2 * proposal_delay_,
          status_factory,
          transaction_factory,
          batch_parser,
          transaction_batch_factory_);

  log_->info("[Init] => command service");
}

/**
 * Initializing query command service
 */
void Irohad::initQueryService() {
  auto query_processor = std::make_shared<QueryProcessorImpl>(
      storage, storage, pending_txs_storage_, query_response_factory_);

  query_service =
      std::make_shared<::torii::QueryService>(query_processor, query_factory);

  log_->info("[Init] => query service");
}

void Irohad::initWsvRestorer() {
  wsv_restorer_ = std::make_shared<iroha::ametsuchi::WsvRestorerImpl>();
}

/**
 * Run iroha daemon
 */
Irohad::RunResult Irohad::run() {
  using iroha::expected::operator|;

  // Initializing torii server
  torii_server = std::make_unique<ServerRunner>(
      listen_ip_ + ":" + std::to_string(torii_port_), false);

  // Initializing internal server
  internal_server = std::make_unique<ServerRunner>(
      listen_ip_ + ":" + std::to_string(internal_port_), false);

  // Run torii server
  return (torii_server->append(command_service_transport)
              .append(query_service)
              .run()
          |
          [&](const auto &port) {
            log_->info("Torii server bound on port {}", port);
            if (is_mst_supported_) {
              internal_server->append(
                  std::static_pointer_cast<MstTransportGrpc>(mst_transport));
            }
            // Run internal server
            return internal_server
                ->append(ordering_init.ordering_gate_transport)
                .append(ordering_init.ordering_service_transport)
                .append(yac_init.consensus_network)
                .append(loader_init.service)
                .run();
          })
      .match(
          [&](const auto &port) -> RunResult {
            log_->info("Internal server bound on port {}", port.value);
            log_->info("===> iroha initialized");
            return {};
          },
          [&](const expected::Error<std::string> &e) -> RunResult {
            log_->error(e.error);
            return e;
          });
}

Irohad::~Irohad() {
  // TODO andrei 17.09.18: IR-1710 Verify that all components' destructors are
  // called in irohad destructor
  storage->freeConnections();
}
