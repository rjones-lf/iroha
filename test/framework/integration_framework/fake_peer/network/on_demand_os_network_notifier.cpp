/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/network/on_demand_os_network_notifier.hpp"
#include "backend/protobuf/proposal.hpp"
#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"
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
        rounds_subject_.get_subscriber().on_next(round);
        auto fake_peer = fake_peer_wptr_.lock();
        BOOST_ASSERT_MSG(fake_peer, "Fake peer shared pointer is not set!");
        const auto behaviour = fake_peer->getBehaviour();
        if (behaviour) {
          auto opt_proposal = behaviour->processOrderingProposalRequest(round);
          if (opt_proposal) {
            return std::unique_ptr<shared_model::interface::Proposal>(
                std::make_unique<shared_model::proto::Proposal>(
                    (*opt_proposal)->getTransport()));
          }
        }
        return {};
      }

      rxcpp::observable<OnDemandOsNetworkNotifier::Round>
      OnDemandOsNetworkNotifier::getProposalRequestsObservable() {
        return rounds_subject_.get_observable();
      }

      rxcpp::observable<std::shared_ptr<BatchesForRound>>
      OnDemandOsNetworkNotifier::getBatchesObservable() {
        return batches_subject_.get_observable();
      }

  }  // namespace fake_peer
}  // namespace integration_framework
