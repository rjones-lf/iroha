/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_H
#define IROHA_ON_DEMAND_ORDERING_SERVICE_H

#include "logger/logger.hpp"
#include "network/ordering_service.hpp"

namespace iroha {
  namespace ordering {

    class OnDemandOrderingService : public network::OrderingService {
      void onTransaction(std::shared_ptr<shared_model::interface::Transaction>
                             transaction) override;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_H
