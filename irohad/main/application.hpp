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

#include <uvw/loop.hpp>

#include "ametsuchi/impl/storage_impl.hpp"
#include "crypto/crypto.hpp"
#include "logger/logger.hpp"
#include "main/impl/block_loader_init.hpp"
#include "main/impl/consensus_init.hpp"
#include "main/impl/ordering_init.hpp"
#include "main/server_runner.hpp"
#include "model/model_crypto_provider_impl.hpp"
#include "model/model_hash_provider_impl.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "simulator/block_creator.hpp"
#include "simulator/impl/simulator.hpp"
#include "synchronizer/synchronizer.hpp"
#include "torii/command_service.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "validation/chain_validator.hpp"
#include "validation/impl/stateless_validator_impl.hpp"
#include "validation/stateful_validator.hpp"

class Irohad {
 public:

  /**
   * Constructor that initializes common iroha pipeline
   * @param block_store_dir - folder where blocks will be stored
   * @param redis_host - host of redis connection
   * @param redis_port - port of redis connection
   * @param pg_conn - initialization string for postgre
   * @param torii_port - port for torii binding
   * @param peer_number - number of peer in ledger // todo replace with pub key
   */
  Irohad(const std::string &block_store_dir, const std::string &redis_host,
         size_t redis_port, const std::string &pg_conn, size_t torii_port,
         uint64_t peer_number);
  void run();
  ~Irohad();

 private:
  std::shared_ptr<iroha::synchronizer::Synchronizer> createSynchronizer(
      std::shared_ptr<iroha::network::ConsensusGate> consensus_gate,
      std::shared_ptr<iroha::validation::ChainValidator> validator,
      std::shared_ptr<iroha::ametsuchi::MutableFactory> mutableFactory,
      std::shared_ptr<iroha::network::BlockLoader> blockLoader);

  std::shared_ptr<iroha::simulator::Simulator> createSimulator(
      std::shared_ptr<iroha::network::OrderingGate> ordering_gate,
      std::shared_ptr<iroha::validation::StatefulValidator> stateful_validator,
      std::shared_ptr<iroha::ametsuchi::BlockQuery> block_query,
      std::shared_ptr<iroha::ametsuchi::TemporaryFactory> temporary_factory,
      std::shared_ptr<iroha::model::HashProviderImpl> hash_provider);

  std::shared_ptr<iroha::network::PeerCommunicationService>
  createPeerCommunicationService(
      std::shared_ptr<iroha::network::OrderingGate> ordering_gate,
      std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer);

  std::unique_ptr<::torii::CommandService> createCommandService(
      std::shared_ptr<iroha::model::converters::PbTransactionFactory>
      pb_factory,
      std::shared_ptr<iroha::torii::TransactionProcessor> txProccesor);

  std::unique_ptr<::torii::QueryService> createQueryService(
      std::shared_ptr<iroha::model::converters::PbQueryFactory>
      pb_query_factory,
      std::shared_ptr<iroha::model::converters::PbQueryResponseFactory>
      pb_query_response_factory,
      std::shared_ptr<iroha::torii::QueryProcessor> query_processor);

  std::shared_ptr<iroha::torii::QueryProcessor> createQueryProcessor(
      std::unique_ptr<iroha::model::QueryProcessingFactory> qpf,
      std::shared_ptr<iroha::validation::StatelessValidator>
      stateless_validator);

  std::shared_ptr<iroha::torii::TransactionProcessor>
  createTransactionProcessor(
      std::shared_ptr<iroha::network::PeerCommunicationService> pcs,
      std::shared_ptr<iroha::validation::StatelessValidator> validator);

  std::shared_ptr<iroha::validation::StatelessValidator>
  createStatelessValidator(
      std::shared_ptr<iroha::model::ModelCryptoProvider> crypto_provider);

  std::unique_ptr<iroha::model::QueryProcessingFactory>
  createQueryProcessingFactory(
      std::shared_ptr<iroha::ametsuchi::WsvQuery> wsvQuery,
      std::shared_ptr<iroha::ametsuchi::BlockQuery> blockQuery);

  std::string block_store_dir_;
  std::string redis_host_;
  size_t redis_port_;
  std::string pg_conn_;
  size_t torii_port_;
  std::shared_ptr<uvw::Loop> loop;

  std::unique_ptr<::torii::CommandService> command_service;
  std::unique_ptr<::torii::QueryService> query_service;

  std::unique_ptr<ServerRunner> torii_server;
  std::unique_ptr<grpc::Server> internal_server;
  iroha::network::OrderingInit ordering_init;
  iroha::consensus::yac::YacInit yac_init;
  iroha::network::BlockLoaderInit loader_init;

  std::thread internal_thread, server_thread;

  logger::Logger log_;

 public:
  std::shared_ptr<iroha::ametsuchi::StorageImpl> storage;
  uint64_t peer_number_;
};

#endif  // IROHA_APPLICATION_HPP
