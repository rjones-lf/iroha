/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "main/application.hpp"

#include <gmock/gmock.h>

#include "ametsuchi/impl/peer_query_wsv.hpp"
#include "network/impl/peer_communication_service_impl.hpp"
#include "synchronizer/impl/synchronizer_impl.hpp"
#include "validation/impl/chain_validator_impl.hpp"
#include "validation/impl/stateful_validator_impl.hpp"

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

Irohad::Irohad(const std::string &block_store_dir,
               const std::string &redis_host, size_t redis_port,
               const std::string &pg_conn, size_t torii_port,
               uint64_t peer_number)
    : block_store_dir_(block_store_dir),
      redis_host_(redis_host),
      redis_port_(redis_port),
      pg_conn_(pg_conn),
      torii_port_(torii_port),
      storage(StorageImpl::create(block_store_dir, redis_host, redis_port,
                                  pg_conn)),
      peer_number_(peer_number) {
  log_ = logger::log("IROHAD");
  log_->info("created");
}

Irohad::~Irohad() {
  internal_server->Shutdown();
  torii_server->shutdown();
  internal_thread.join();
  server_thread.join();
}

class MockCryptoProvider : public ModelCryptoProvider {
 public:
  MOCK_CONST_METHOD1(verify, bool(
      const Transaction &));
  MOCK_CONST_METHOD1(verify, bool(std::shared_ptr<const Query>));
  MOCK_CONST_METHOD1(verify, bool(
      const Block &));
};

void Irohad::run() {
  loop = uvw::Loop::create();

  torii_server =
      std::make_unique<ServerRunner>("0.0.0.0:" + std::to_string(torii_port_));

  // Protobuf converters
  auto pb_tx_factory = std::make_shared<PbTransactionFactory>();
  auto pb_query_factory = std::make_shared<PbQueryFactory>();
  auto pb_query_response_factory = std::make_shared<PbQueryResponseFactory>();
  log_->info("[Init] => converters");

  // Crypto Provider:
  auto crypto_verifier = std::make_shared<MockCryptoProvider>();

  EXPECT_CALL(*crypto_verifier, verify(::testing::A<const Transaction &>()))
      .WillRepeatedly(::testing::Return(true));
  EXPECT_CALL(*crypto_verifier,
              verify(::testing::A<std::shared_ptr<const Query>>()))
      .WillRepeatedly(::testing::Return(true));
  EXPECT_CALL(*crypto_verifier, verify(::testing::A<const Block &>()))
      .WillRepeatedly(::testing::Return(true));
  log_->info("[Init] => crypto provider");

  // Hash provider
  auto hash_provider = std::make_shared<HashProviderImpl>();
  log_->info("[Init] => hash provider");

  // Validators:
  auto stateless_validator = createStatelessValidator(crypto_verifier);
  auto stateful_validator = std::make_shared<StatefulValidatorImpl>();
  auto chain_validator = std::make_shared<ChainValidatorImpl>(crypto_verifier);
  log_->info("[Init] => validators");

  auto wsv = std::make_shared<ametsuchi::PeerQueryWsv>(storage);

  auto orderer = std::make_shared<PeerOrdererImpl>(wsv);
  log_->info("[Init] => peer orderer");

  auto peer_address = wsv->getLedgerPeers().value().at(peer_number_).address;

  // Ordering gate
  auto ordering_gate = ordering_init.initOrderingGate(wsv, loop, 10, 5000);
  log_->info("[Init] => init ordering gate - [{}]",
             logger::logBool(ordering_gate));

  // Simulator
  auto simulator = createSimulator(ordering_gate, stateful_validator, storage,
                                   storage, hash_provider);

  // Block loader
  auto block_loader = loader_init.initBlockLoader(wsv, storage);

  // Consensus gate
  auto consensus_gate = yac_init.initConsensusGate(peer_address,
                                                   loop,
                                                   orderer,
                                                   simulator,
                                                   block_loader);

  // Synchronizer
  auto synchronizer = createSynchronizer(consensus_gate, chain_validator,
                                         storage, block_loader);

  // PeerCommunicationService
  auto pcs = createPeerCommunicationService(ordering_gate, synchronizer);

  pcs->on_proposal().subscribe([this](auto) {
    log_->info("~~~~~~~~~| PROPOSAL ^_^ |~~~~~~~~~ ");
  });

  pcs->on_commit().subscribe([this](auto) {
    log_->info("~~~~~~~~~| COMMIT =^._.^= |~~~~~~~~~ ");
  });

  // Torii:
  // --- Transactions:
  auto tx_processor = createTransactionProcessor(pcs, stateless_validator);

  command_service = createCommandService(pb_tx_factory, tx_processor);

  // --- Queries
  auto query_proccessing_factory =
      createQueryProcessingFactory(storage, storage);

  auto query_processor = createQueryProcessor(
      std::move(query_proccessing_factory), stateless_validator);

  query_service = createQueryService(
      pb_query_factory, pb_query_response_factory, query_processor);

  grpc::ServerBuilder builder;
  int port = 0;
  builder.AddListeningPort(peer_address,
                           grpc::InsecureServerCredentials(), &port);
  builder.RegisterService(ordering_init.ordering_gate.get());
  builder.RegisterService(ordering_init.ordering_service.get());
  builder.RegisterService(yac_init.consensus_network.get());
  builder.RegisterService(loader_init.service.get());
  internal_server = builder.BuildAndStart();
  internal_thread = std::thread([this] { internal_server->Wait(); });
  server_thread = std::thread([this] {
    torii_server->run(std::move(command_service), std::move(query_service));
  });
  log_->info("===> iroha initialized");
  torii_server->waitForServersReady();
  loop->run();
}

std::shared_ptr<Simulator> Irohad::createSimulator(
    std::shared_ptr<OrderingGate> ordering_gate,
    std::shared_ptr<StatefulValidator> stateful_validator,
    std::shared_ptr<BlockQuery> block_query,
    std::shared_ptr<TemporaryFactory> temporary_factory,
    std::shared_ptr<HashProviderImpl> hash_provider) {
  return std::make_shared<Simulator>(ordering_gate, stateful_validator,
                                     temporary_factory, block_query,
                                     hash_provider);
}

std::shared_ptr<PeerCommunicationService>
Irohad::createPeerCommunicationService(
    std::shared_ptr<OrderingGate> ordering_gate,
    std::shared_ptr<Synchronizer> synchronizer) {
  return std::make_shared<PeerCommunicationServiceImpl>(ordering_gate,
                                                        synchronizer);
}

std::shared_ptr<Synchronizer> Irohad::createSynchronizer(
    std::shared_ptr<ConsensusGate> consensus_gate,
    std::shared_ptr<ChainValidator> validator,
    std::shared_ptr<MutableFactory> mutableFactory,
    std::shared_ptr<BlockLoader> blockLoader) {
  return std::make_shared<SynchronizerImpl>(consensus_gate, validator,
                                            mutableFactory, blockLoader);
}

std::unique_ptr<::torii::CommandService> Irohad::createCommandService(
    std::shared_ptr<PbTransactionFactory> pb_factory,
    std::shared_ptr<TransactionProcessor> txProccesor) {
  return std::make_unique<::torii::CommandService>(pb_factory, txProccesor);
}

std::unique_ptr<::torii::QueryService> Irohad::createQueryService(
    std::shared_ptr<PbQueryFactory> pb_query_factory,
    std::shared_ptr<PbQueryResponseFactory> pb_query_response_factory,
    std::shared_ptr<QueryProcessor> query_processor) {
  return std::make_unique<::torii::QueryService>(
      pb_query_factory, pb_query_response_factory, query_processor);
}

std::shared_ptr<QueryProcessor> Irohad::createQueryProcessor(
    std::unique_ptr<QueryProcessingFactory> qpf,
    std::shared_ptr<StatelessValidator> stateless_validator) {
  return std::make_shared<QueryProcessorImpl>(std::move(qpf),
                                              stateless_validator);
}

std::shared_ptr<TransactionProcessor> Irohad::createTransactionProcessor(
    std::shared_ptr<PeerCommunicationService> pcs,
    std::shared_ptr<StatelessValidator> validator) {
  return std::make_shared<TransactionProcessorImpl>(pcs, validator);
}

std::shared_ptr<StatelessValidator> Irohad::createStatelessValidator(
    std::shared_ptr<ModelCryptoProvider> crypto_provider) {
  return std::make_shared<StatelessValidatorImpl>(crypto_provider);
}

std::unique_ptr<QueryProcessingFactory> Irohad::createQueryProcessingFactory(
    std::shared_ptr<WsvQuery> wsvQuery,
    std::shared_ptr<BlockQuery> blockQuery) {
  return std::make_unique<QueryProcessingFactory>(wsvQuery, blockQuery);
}
