/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MOCK_ON_DEMAND_OS_NOTIFICATION_HPP
#define IROHA_MOCK_ON_DEMAND_OS_NOTIFICATION_HPP

#include "ordering/on_demand_os_transport.hpp"

#include <gmock/gmock.h>

namespace iroha {
  namespace ordering {
    namespace transport {

      struct MockOdOsNotification : public OdOsNotification {
        MOCK_METHOD2(onBatches, void(consensus::Round, CollectionType));

        MOCK_METHOD1(onRequestProposal,
                     boost::optional<ProposalType>(consensus::Round));
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_MOCK_ON_DEMAND_OS_NOTIFICATION_HPP
