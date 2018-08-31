/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ORDERING_MOCKS_HPP
#define IROHA_ORDERING_MOCKS_HPP

#include <gmock/gmock.h>

#include "ordering/on_demand_ordering_service.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace ordering {
    namespace transport {

      struct MockOdOsNotification : public OdOsNotification {
        MOCK_METHOD2(onTransactions, void(Round, CollectionType));

        MOCK_METHOD1(onRequestProposal, boost::optional<ProposalType>(Round));
      };

      struct MockOdOsNotificationFactory : public OdOsNotificationFactory {
        MOCK_METHOD1(create,
                     std::unique_ptr<OdOsNotification>(
                         const shared_model::interface::Peer &));
      };

    }  // namespace transport

    struct MockOnDemandOrderingService : public OnDemandOrderingService {
      MOCK_METHOD2(onTransactions, void(transport::Round, CollectionType));

      MOCK_METHOD1(onRequestProposal,
                   boost::optional<ProposalType>(transport::Round));

      MOCK_METHOD1(onCollaborationOutcome, void(transport::Round));
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ORDERING_MOCKS_HPP
