/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_service_impl.hpp"

using namespace iroha::ordering;

// ------------------ | OnDemandOrderingService |-------------------------

void OnDemandOrderingServiceImpl::onCollaborationOutcome(
    RoundOutput outcome, transport::RoundType round) {
  throw std::logic_error("Function not yet implemented");
}

// -------------------- | OdOsNotification |------------------------------

void OnDemandOrderingServiceImpl::onTransactions(
    std::vector<std::shared_ptr<shared_model::interface::Transaction>>
        transactions) {
  throw std::logic_error("Function not yet implemented");
}

boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingServiceImpl::onRequestProposal(uint64_t round) {
  throw std::logic_error("Function not yet implemented");
}
