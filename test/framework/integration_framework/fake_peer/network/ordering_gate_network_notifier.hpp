/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_

#include "network/ordering_gate_transport.hpp"

#include <rxcpp/rx.hpp>

#include "framework/integration_framework/fake_peer/types.hpp"

namespace integration_framework {
  namespace fake_peer {

    class OgNetworkNotifier final
        : public iroha::network::OrderingGateNotification {
     public:
      void onProposal(OgProposalSPtr proposal) override;

      rxcpp::observable<OgProposalSPtr> getObservable();

     private:
      rxcpp::subjects::subject<OgProposalSPtr> proposals_subject_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_ */
