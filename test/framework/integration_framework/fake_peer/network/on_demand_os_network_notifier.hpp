/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_ODOS_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_ODOS_NETWORK_NOTIFIER_HPP_

#include <rxcpp/rx.hpp>

#include "consensus/round.hpp"
#include "framework/integration_framework/fake_peer/network/batches_for_round.hpp"
#include "framework/integration_framework/fake_peer/types.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace integration_framework {
  namespace fake_peer {

    class OnDemandOsNetworkNotifier final
        : public iroha::ordering::transport::OdOsNotification {
     public:
      using Round = iroha::consensus::Round;

      OnDemandOsNetworkNotifier(const std::shared_ptr<FakePeer> &fake_peer);

      virtual void onBatches(Round round, CollectionType batches);

      virtual boost::optional<ProposalType> onRequestProposal(Round round);

      rxcpp::observable<Round> get_proposal_requests_observable();

      rxcpp::observable<std::shared_ptr<BatchesForRound>>
      get_batches_observable();

     private:
      std::weak_ptr<FakePeer> fake_peer_wptr_;
      rxcpp::subjects::subject<Round> rounds_subject_;
      rxcpp::subjects::subject<std::shared_ptr<BatchesForRound>>
          batches_subject_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* FAKE_PEER_ODOS_NETWORK_NOTIFIER_HPP_ */
