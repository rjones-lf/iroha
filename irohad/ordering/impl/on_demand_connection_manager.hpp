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
       * Responsibilities of individual peers from the peers array
       * Transactions are sent to three ordering services:
       * reject round for current block, reject round for next block, and
       * commit for subsequent next round
       * Proposal is requested from the current ordering service: issuer
       */
      enum PeerType {
        kCurrentRoundRejectConsumer = 0,
        kNextRoundRejectConsumer,
        kNextRoundCommitConsumer,
        kIssuer,
        kCount
      };

      /// Collection with value types which represent peers
      template <typename T>
      using PeerCollectionType = std::array<T, kCount>;

      /**
       * Current peers to send transactions and request proposals
       * @see PeerType for individual descriptions
       */
      struct CurrentPeers {
        PeerCollectionType<std::shared_ptr<shared_model::interface::Peer>>
            peers;
      };

      OnDemandConnectionManager(
          std::shared_ptr<transport::OdOsNotificationFactory> factory,
          CurrentPeers initial_peers,
          rxcpp::observable<CurrentPeers> peers);

      void onTransactions(transport::Round round,
                          CollectionType transactions) override;

      boost::optional<ProposalType> onRequestProposal(
          transport::Round round) override;

     private:
      /**
       * Corresponding connections created by OdOsNotificationFactory
       * @see PeerType for individual descriptions
       */
      struct CurrentConnections {
        PeerCollectionType<std::unique_ptr<transport::OdOsNotification>> peers;
      };

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
