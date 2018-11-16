/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/network/on_demand_os_network_notifier.hpp"
#include "backend/protobuf/proposal.hpp"
#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "framework/integration_framework/fake_peer/proposal_storage.hpp"

namespace integration_framework {
  namespace fake_peer {

      OnDemandOsNetworkNotifier::OnDemandOsNetworkNotifier(
          const std::shared_ptr<FakePeer> &fake_peer)
          : fake_peer_wptr_(fake_peer) {}

      void OnDemandOsNetworkNotifier::onBatches(Round round,
                                                CollectionType batches) {
        batches_subject_.get_subscriber().on_next(
            std::make_shared<BatchesForRound>(round, batches));
      }

      boost::optional<OnDemandOsNetworkNotifier::ProposalType>
      OnDemandOsNetworkNotifier::onRequestProposal(Round round) {
        auto fake_peer = fake_peer_wptr_.lock();
        BOOST_ASSERT_MSG(fake_peer, "Fake peer shared pointer is not set!");
        const auto proposal_storage = fake_peer->getProposalStorage();
        if (proposal_storage) {
          auto proposal = proposal_storage->getProposal(round);
          if (proposal) {
            return std::unique_ptr<shared_model::interface::Proposal>(
                std::make_unique<shared_model::proto::Proposal>(
                    proposal->getTransport()));
          }
        }
        return {};
      }

      rxcpp::observable<OnDemandOsNetworkNotifier::Round>
      OnDemandOsNetworkNotifier::get_proposal_requests_observable() {
        return rounds_subject_.get_observable();
      }

      rxcpp::observable<std::shared_ptr<BatchesForRound>>
      OnDemandOsNetworkNotifier::get_batches_observable() {
        return batches_subject_.get_observable();
      }

  }  // namespace fake_peer
}  // namespace integration_framework
