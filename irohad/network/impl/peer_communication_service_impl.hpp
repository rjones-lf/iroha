/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP
#define IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP

#include "network/ordering_gate.hpp"
#include "network/peer_communication_service.hpp"
#include "simulator/verified_proposal_creator.hpp"
#include "synchronizer/synchronizer.hpp"
#include "validation/stateful_validator_common.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace network {
    class PeerCommunicationServiceImpl : public PeerCommunicationService {
     public:
      PeerCommunicationServiceImpl(
          std::shared_ptr<OrderingGate> ordering_gate,
          std::shared_ptr<synchronizer::Synchronizer> synchronizer,
          std::shared_ptr<simulator::VerifiedProposalCreator> proposal_creator);

      void propagate_batch(
          std::shared_ptr<shared_model::interface::TransactionBatch> batch)
          const override;

      rxcpp::observable<OrderingEvent> onProposal() const override;

      rxcpp::observable<
          std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      onVerifiedProposal() const override;

      rxcpp::observable<synchronizer::SynchronizationEvent> on_commit()
          const override;

     private:
      std::shared_ptr<OrderingGate> ordering_gate_;
      std::shared_ptr<synchronizer::Synchronizer> synchronizer_;
      std::shared_ptr<simulator::VerifiedProposalCreator> proposal_creator_;
      logger::Logger log_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_PEER_COMMUNICATION_SERVICE_IMPL_HPP
