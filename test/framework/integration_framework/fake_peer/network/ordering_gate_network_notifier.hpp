/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_

#include "network/ordering_gate_transport.hpp"

#include <rxcpp/rx.hpp>

namespace shared_model {
  namespace interface {
    class Proposal;
  }
}  // namespace shared_model

namespace integration_framework {

  class OgNetworkNotifier final
      : public iroha::network::OrderingGateNotification {
   public:
    using Proposal = shared_model::interface::Proposal;
    using ProposalPtr = std::shared_ptr<Proposal>;

    void onProposal(ProposalPtr proposal) override;

    rxcpp::observable<ProposalPtr> get_observable();

   private:
    rxcpp::subjects::subject<ProposalPtr> proposals_subject_;
  };

}  // namespace integration_framework

#endif /* FAKE_PEER_OG_NETWORK_NOTIFIER_HPP_ */
