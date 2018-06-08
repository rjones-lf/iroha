/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_H
#define IROHA_ON_DEMAND_ORDERING_SERVICE_H

#include "logger/logger.hpp"
#include "network/ordering_service.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace ordering {

    /**
     * Enum type with outcome of consensus on proposals
     */
    enum class RoundOutput { SUCCESSFUL, REJECT };

    /**
     * Ordering Service aka OS which can share proposals by request
     */
    class OnDemandOrderingService : public transport::OdOsNotification {
     public:
      /**
       * Method which should be invoked on outcome of collaboration for round
       * @param outcome - status of collaboration on proposal
       * @param round - number of round
       */
      virtual void onCollaborationOutcome(RoundOutput outcome,
                                          transport::RoundType round) = 0;

      virtual ~OnDemandOrderingService() = default;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_H
