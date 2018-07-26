/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_HPP
#define IROHA_ON_DEMAND_ORDERING_SERVICE_HPP

#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace ordering {

    /**
     * Enum type with outcomes of proposal and block consensus
     */
    enum class RoundOutput { kCommitProposal, kCommitBlock, kReject };

    /**
     * Ordering Service aka OS which can share proposals by request
     */
    class OnDemandOrderingService : public transport::OdOsNotification {
     public:
      /**
       * Method which should be invoked on outcome of collaboration for round
       * @param outcome - status of collaboration on proposal
       */
      virtual void onCollaborationOutcome(RoundOutput outcome) = 0;

      virtual ~OnDemandOrderingService() = default;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_HPP
