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

#ifndef IROHA_NETWORK_MOCKS_HPP
#define IROHA_NETWORK_MOCKS_HPP

#include <gmock/gmock.h>
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"

namespace iroha {
  namespace network {
    class MockPeerCommunicationService : public PeerCommunicationService {
     public:
      MOCK_METHOD1(
          propagate_transaction,
          void(std::shared_ptr<const shared_model::interface::Transaction>));

      MOCK_CONST_METHOD0(
          on_proposal,
          rxcpp::observable<
              std::shared_ptr<shared_model::interface::Proposal>>());

      MOCK_CONST_METHOD0(on_commit, rxcpp::observable<Commit>());
    };

    class MockBlockLoader : public BlockLoader {
     public:
      MOCK_METHOD1(retrieveBlocks,
                   rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>(
                       const shared_model::crypto::PublicKey &));
      MOCK_METHOD2(retrieveBlock,
                   boost::optional<std::shared_ptr<shared_model::interface::Block>>(
                       const shared_model::crypto::PublicKey &,
                       const shared_model::interface::types::HashType &));
    };

    class MockOrderingGate : public OrderingGate {
     public:
      MOCK_METHOD1(propagateTransaction,
                   void(std::shared_ptr<const model::Transaction> transaction));

      MOCK_METHOD0(on_proposal, rxcpp::observable<model::Proposal>());

      MOCK_METHOD1(setPcs, void(const PeerCommunicationService &));
    };

    class MockConsensusGate : public ConsensusGate {
     public:
      MOCK_METHOD1(vote, void(model::Block));

      MOCK_METHOD0(on_commit, rxcpp::observable<model::Block>());
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_NETWORK_MOCKS_HPP
