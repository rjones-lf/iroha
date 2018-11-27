/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/ordering_gate_network_notifier.hpp"

#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace integration_framework {

  void OgNetworkNotifier::onProposal(OgNetworkNotifier::ProposalPtr proposal) {
    proposals_subject_.get_subscriber().on_next(std::move(proposal));
  }

  rxcpp::observable<OgNetworkNotifier::ProposalPtr>
  OgNetworkNotifier::get_observable() {
    return proposals_subject_.get_observable();
  }

}  // namespace integration_framework
