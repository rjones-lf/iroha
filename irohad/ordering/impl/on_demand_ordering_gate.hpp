/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_GATE_HPP
#define IROHA_ON_DEMAND_ORDERING_GATE_HPP

#include "network/ordering_gate.hpp"

#include <shared_mutex>

#include <boost/variant.hpp>
#include <rxcpp/rx.hpp>
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "ordering/impl/ordering_gate_cache/ordering_gate_cache.hpp"
#include "ordering/on_demand_ordering_service.hpp"

namespace iroha {
  namespace ametsuchi {
    class TxPresenceCache;
  }

  namespace ordering {

    /**
     * Ordering gate which requests proposals from the ordering service
     * votes for proposals, and passes committed proposals to the pipeline
     */
    class OnDemandOrderingGate : public network::OrderingGate {
     public:
      /**
       * Represents storage modification. Proposal round increment
       */
      struct BlockEvent {
        /// next round number
        consensus::Round round;
        /// hashes of processed transactions
        cache::OrderingGateCache::HashesSetType hashes;
      };

      /**
       * Represents no storage modification. Reject round increment
       */
      struct EmptyEvent {
        /// next round number
        consensus::Round round;
      };

      using BlockRoundEventType = boost::variant<BlockEvent, EmptyEvent>;

      OnDemandOrderingGate(
          std::shared_ptr<OnDemandOrderingService> ordering_service,
          std::shared_ptr<transport::OdOsNotification> network_client,
          rxcpp::observable<BlockRoundEventType> events,
          std::shared_ptr<cache::OrderingGateCache>
              cache,  // TODO: IR-1863 12.11.18 kamilsa change cache to
                      // unique_ptr
          std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
              factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
          consensus::Round initial_round,
          logger::Logger log = logger::log("OnDemandOrderingGate"));

      ~OnDemandOrderingGate() override;

      void propagateBatch(
          std::shared_ptr<shared_model::interface::TransactionBatch> batch)
          override;

      rxcpp::observable<network::OrderingEvent> onProposal() override;

      [[deprecated("Use ctor")]] void setPcs(
          const iroha::network::PeerCommunicationService &pcs) override;

     private:
      /**
       * Handle an incoming proposal from ordering service
       */
      boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
      processProposalRequest(
          boost::optional<OnDemandOrderingService::ProposalType> &&proposal)
          const;

      /**
       * remove already processed transactions from proposal
       */
      boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
      removeReplays(shared_model::interface::Proposal &&proposal) const;

      logger::Logger log_;
      std::shared_ptr<OnDemandOrderingService> ordering_service_;
      std::shared_ptr<transport::OdOsNotification> network_client_;
      rxcpp::composite_subscription events_subscription_;
      std::shared_ptr<cache::OrderingGateCache> cache_;
      std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
          proposal_factory_;
      std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache_;

      consensus::Round current_round_;
      rxcpp::subjects::subject<network::OrderingEvent> proposal_notifier_;
      mutable std::shared_timed_mutex mutex_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_GATE_HPP
