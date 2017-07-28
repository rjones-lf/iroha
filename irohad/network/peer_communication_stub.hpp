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

#include <ametsuchi/storage.hpp>
#include <consensus/consensus_service_stub.hpp>
#include <model/model_crypto_provider.hpp>
#include <model/model_hash_provider.hpp>
#include <network/ordering_gate.hpp>
#include <network/peer_communication_service.hpp>
#include <ordering/ordering_service_stub.hpp>
#include <validation/chain_validator.hpp>
#include <validation/stateful_validator.hpp>

namespace iroha {
  namespace network {
    class PeerCommunicationServiceStub : public PeerCommunicationService {
     public:
      PeerCommunicationServiceStub(network::OrderingGate& orderer,
                                   consensus::ConsensusService& consensus);

      void propagate_transaction(model::Transaction transaction);

      rxcpp::observable<model::Proposal> on_proposal() override;

      rxcpp::observable<rxcpp::observable<model::Block>> on_commit() override;

     private:
      network::OrderingGate& orderer_;
      consensus::ConsensusService& consensus_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_PEER_COMMUNICATION_STUB_HPP
