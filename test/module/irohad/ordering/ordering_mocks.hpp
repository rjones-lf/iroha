/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ORDERING_MOCKS_HPP
#define IROHA_ORDERING_MOCKS_HPP

#include <gmock/gmock.h>

#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace ordering {
    namespace transport {

      struct MockOdOsNotification : public OdOsNotification {
        MOCK_METHOD2(onTransactions, void(RoundType, CollectionType));

        MOCK_METHOD1(onRequestProposal,
                     boost::optional<ProposalType>(RoundType));
      };

      struct MockOdOsNotificationFactory : public OdOsNotificationFactory {
        MOCK_METHOD1(create,
                     std::unique_ptr<OdOsNotification>(
                         const shared_model::interface::Peer &));
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_MOCKS_HPP
