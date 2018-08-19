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
      /**
       * Current peers to send transactions and request proposals
       * Transactions are sent to two ordering services:
       * current and previous consumers
       * Proposal is requested from current ordering service: issuer
       */
      struct CurrentPeers {
        std::shared_ptr<shared_model::interface::Peer> issuer, current_consumer,
            previous_consumer;
      };

      /**
       * Corresponding connections created by OdOsNotificationFactory
       * @see CurrentPeers for individual descriptions
       */
      struct CurrentConnections {
        std::unique_ptr<transport::OdOsNotification> issuer, current_consumer,
            previous_consumer;
      };

      OnDemandConnectionManager(
          std::shared_ptr<transport::OdOsNotificationFactory> factory,
          CurrentPeers initial_peers,
          rxcpp::observable<CurrentPeers> peers);

      void onTransactions(CollectionType transactions) override;

      boost::optional<ProposalType> onRequestProposal(
          transport::RoundType round) override;

     private:
      /**
       * Initialize corresponding peers in connections_ using factory_
       * @param peers to initialize connections with
       */
      void initializeConnections(const CurrentPeers &peers);

      std::shared_ptr<transport::OdOsNotificationFactory> factory_;
      rxcpp::composite_subscription subscription_;

      CurrentConnections connections_;

      std::shared_timed_mutex mutex_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_CONNECTION_MANAGER_HPP
