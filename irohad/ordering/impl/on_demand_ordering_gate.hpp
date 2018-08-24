/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_GATE_HPP
#define IROHA_ON_DEMAND_ORDERING_GATE_HPP

#include "network/ordering_gate.hpp"

#include <shared_mutex>

#include <rxcpp/rx-observable.hpp>
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "network/proposal_gate.hpp"
#include "ordering/on_demand_ordering_service.hpp"

namespace iroha {
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
        shared_model::interface::types::HeightType height;
      };

      /**
       * Represents no storage modification. Reject round increment
       */
      struct EmptyEvent {};

      using BlockRoundEventType = boost::variant<BlockEvent, EmptyEvent>;

      OnDemandOrderingGate(
          std::shared_ptr<OnDemandOrderingService> ordering_service,
          std::shared_ptr<transport::OdOsNotification> network_client,
          rxcpp::observable<BlockRoundEventType> events,
          std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              factory,
          transport::RoundType initial_round);

      void propagateTransaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) const override;

      void propagateBatch(const shared_model::interface::TransactionBatch
                              &batch) const override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() override;

      void setPcs(const iroha::network::PeerCommunicationService &pcs) override;

     private:
      /**
       * Update the local ordering service, request the proposal and vote for it
       */
      void vote();

      std::shared_ptr<OnDemandOrderingService> ordering_service_;
      std::shared_ptr<transport::OdOsNotification> network_client_;
      rxcpp::composite_subscription events_subscription_;
      std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
          proposal_factory_;

      transport::RoundType current_round_;
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::Proposal>>
          proposal_notifier_;
      mutable std::shared_timed_mutex mutex_;
    };

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_GATE_HPP
