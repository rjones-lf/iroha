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
#ifndef IROHA_PEER_COMMUNICATION_STUB_HPP
#define IROHA_PEER_COMMUNICATION_STUB_HPP

#include <network/network_api.h>
#include <ametsuchi/ametsuchi.hpp>
#include <consensus/consensus_service.hpp>
#include <ordering/ordering_service.hpp>
#include <validation/chain/validator.hpp>
#include <validation/stateful/validator.hpp>
#include <dao/dao_crypto_provider.hpp>
#include <dao/dao_hash_provider.hpp>

namespace iroha {
  namespace network {
    class PeerCommunicationServiceStub : public PeerCommunicationService {
     public:
      PeerCommunicationServiceStub(
          ametsuchi::Ametsuchi &storage,
          validation::StatefulValidator &stateful_validator,
          validation::ChainValidator &chain_validator,
          ordering::OrderingService &orderer,
          consensus::ConsensusService &consensus,
          dao::DaoCryptoProvider &crypto_provider);

      rxcpp::observable<dao::Proposal> on_proposal() override;

      rxcpp::observable<rxcpp::observable<dao::Block>> on_commit() override;

      void propagate_transaction(const dao::Transaction &tx) override;

      void subscribe_on_proposal();

     private:
      ametsuchi::Ametsuchi &storage_;
      validation::StatefulValidator &stateful_validator_;
      validation::ChainValidator &chain_validator_;
      ordering::OrderingService &orderer_;
      consensus::ConsensusService &consensus_;
      dao::DaoCryptoProvider &crypto_provider_;
      // TODO add hash provider
    };
  }
}

#endif  // IROHA_PEER_COMMUNICATION_STUB_HPP
