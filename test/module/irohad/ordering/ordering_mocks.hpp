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
        MOCK_METHOD1(doOnTransactions, void(CollectionType &transactions));

        void onTransactions(CollectionType &&transactions) {
          doOnTransactions(transactions);
        }

        MOCK_METHOD1(onRequestProposal,
                     boost::optional<ProposalType>(RoundType round));
      };

      class MockOdOsNotificationFactory : public OdOsNotificationFactory {
        MOCK_METHOD1(create,
                     std::unique_ptr<OdOsNotification>(
                         const shared_model::interface::Peer &to));
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_MOCKS_HPP
