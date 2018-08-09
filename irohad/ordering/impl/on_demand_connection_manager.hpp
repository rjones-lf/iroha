/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_CONNECTION_MANAGER_HPP
#define IROHA_ON_DEMAND_CONNECTION_MANAGER_HPP

#include "ordering/on_demand_os_transport.hpp"

#include <shared_mutex>

#include <rxcpp/rx-observable.hpp>

namespace iroha {
  namespace ordering {

    /**
     * Proxy class which redirects requests to appropriate peers
     */
    class OnDemandConnectionManager : public transport::OdOsNotification {
     public:
      struct CurrentPeers {
        std::shared_ptr<shared_model::interface::Peer> issuer, current_consumer,
            previous_consumer;
      };

      OnDemandConnectionManager(
          std::shared_ptr<transport::OdOsNotificationFactory> factory,
          rxcpp::observable<CurrentPeers> peers);

      // OdOsNotification

      void onTransactions(CollectionType transactions) override;

      boost::optional<ProposalType> onRequestProposal(
          transport::RoundType round) override;

     private:
      std::shared_ptr<transport::OdOsNotificationFactory> factory_;
      rxcpp::composite_subscription subscription_;

      std::unique_ptr<transport::OdOsNotification> issuer_, current_consumer_,
          previous_consumer_;

      std::shared_timed_mutex mutex_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_CONNECTION_MANAGER_HPP
