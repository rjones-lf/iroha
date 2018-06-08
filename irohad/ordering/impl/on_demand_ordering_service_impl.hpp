/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
#define IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP

#include "ordering/on_demand_ordering_service.hpp"

namespace iroha {
  namespace ordering {
    class OnDemandOrderingServiceImpl : public OnDemandOrderingService {
     public:
      // --------------------- | OnDemandOrderingService | ---------------------

      void onCollaborationOutcome(RoundOutput outcome,
                                  transport::RoundType round) override;

      // ----------------------- | OdOsNotification | --------------------------

      void onTransactions(
          std::vector<std::shared_ptr<shared_model::interface::Transaction>>
              transactions) override;

      boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
      onRequestProposal(uint64_t round) override;
    };
  }  // namespace ordering
}  // namespace iroha
#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
