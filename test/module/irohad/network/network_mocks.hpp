/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_NETWORK_MOCKS_HPP
#define IROHA_NETWORK_MOCKS_HPP

#include <gmock/gmock.h>
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "synchronizer/synchronizer_common.hpp"

namespace shared_model {
  namespace interface {
    class Transaction;
  }
}  // namespace shared_model

namespace iroha {
  namespace network {
    class MockPeerCommunicationService : public PeerCommunicationService {
     public:
      MOCK_CONST_METHOD1(
          propagate_transaction,
          void(std::shared_ptr<const shared_model::interface::Transaction>));

      MOCK_CONST_METHOD1(
          propagate_batch,
          void(std::shared_ptr<shared_model::interface::TransactionBatch>));

      MOCK_CONST_METHOD0(
          on_proposal,
          rxcpp::observable<
              std::shared_ptr<shared_model::interface::Proposal>>());

      MOCK_CONST_METHOD0(
          on_commit, rxcpp::observable<synchronizer::SynchronizationEvent>());

      MOCK_CONST_METHOD0(
          on_verified_proposal,
          rxcpp::observable<
              std::shared_ptr<validation::VerifiedProposalAndErrors>>());
    };

    class MockBlockLoader : public BlockLoader {
     public:
      MOCK_METHOD2(
          retrieveBlocks,
          rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>(
              const shared_model::interface::types::HeightType,
              const shared_model::crypto::PublicKey &));
      MOCK_METHOD2(
          retrieveBlock,
          boost::optional<std::shared_ptr<shared_model::interface::Block>>(
              const shared_model::crypto::PublicKey &,
              const shared_model::interface::types::HashType &));
    };

    class MockOrderingGate : public OrderingGate {
     public:
      MOCK_CONST_METHOD1(
          propagateTransaction,
          void(std::shared_ptr<const shared_model::interface::Transaction>
                   transaction));

      MOCK_METHOD1(
          propagateBatch,
          void(std::shared_ptr<shared_model::interface::TransactionBatch>));

      MOCK_METHOD0(on_proposal,
                   rxcpp::observable<
                       std::shared_ptr<shared_model::interface::Proposal>>());

      MOCK_METHOD1(setPcs, void(const PeerCommunicationService &));
    };

    class MockConsensusGate : public ConsensusGate {
     public:
      MOCK_METHOD1(vote, void(std::shared_ptr<shared_model::interface::Block>));

      MOCK_METHOD0(on_commit, rxcpp::observable<Commit>());
    };

  }  // namespace network
}  // namespace iroha

#endif  // IROHA_NETWORK_MOCKS_HPP
