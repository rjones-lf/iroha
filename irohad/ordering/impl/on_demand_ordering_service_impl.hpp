/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
#define IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP

#include "ordering/on_demand_ordering_service.hpp"

#include <queue>
#include <shared_mutex>
#include <unordered_map>

#include <tbb/concurrent_queue.h>
#include "ametsuchi/tx_presence_cache.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "logger/logger.hpp"
#include "common/visitor.hpp"

namespace iroha {
  namespace ordering {
    class OnDemandOrderingServiceImpl : public OnDemandOrderingService {
     public:
      /**
       * Create on_demand ordering service with following options:
       * @param transaction_limit - number of maximum transactions in one
       * proposal
       * @param number_of_proposals - number of stored proposals, older will be
       * removed. Default value is 3
       * @param initial_round - first round of agreement.
       * Default value is {2, 1} since genesis block height is 1
       */
      OnDemandOrderingServiceImpl(
          size_t transaction_limit,
          std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              proposal_factory,
          std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache,
          size_t number_of_proposals = 3,
          const consensus::Round &initial_round = {2, 1});

      // --------------------- | OnDemandOrderingService |_---------------------

      void onCollaborationOutcome(consensus::Round round) override;

      // ----------------------- | OdOsNotification | --------------------------

      void onBatches(consensus::Round, CollectionType batches) override;

      boost::optional<ProposalType> onRequestProposal(
          consensus::Round round) override;

     private:
      /**
       * Packs new proposals and creates new rounds
       * Note: method is not thread-safe
       */
      void packNextProposals(const consensus::Round &round);

      /**
       * Removes last elements if it is required
       * Method removes the oldest commit or chain of the oldest rejects
       * Note: method is not thread-safe
       */
      void tryErase();

      /**
       * @return packed proposal from the given round queue
       * Note: method is not thread-safe
       */
      ProposalType emitProposal(const consensus::Round &round);

      bool batchAlreadyProcessed(
          const shared_model::interface::TransactionBatch &batch) {
        auto result = tx_cache_->check(batch);
        // if all missing, then valid, else invalid
        return std::any_of(
            result.begin(), result.end(), [](const auto &batch_result) {
              return iroha::visit_in_place(
                  batch_result,
                  [](const ametsuchi::tx_cache_status_responses::Missing &) {
                    return false;
                  },
                  [](const auto &) { return true; });
            });
      }

      /**
       * Max number of transaction in one proposal
       */
      size_t transaction_limit_;

      /**
       * Max number of available proposals in one OS
       */
      size_t number_of_proposals_;

      /**
       * Queue which holds all rounds in linear order
       */
      std::queue<consensus::Round> round_queue_;

      /**
       * Map of available proposals
       */
      std::unordered_map<consensus::Round,
                         ProposalType,
                         consensus::RoundTypeHasher>
          proposal_map_;

      /**
       * Proposals for current rounds
       */
      std::unordered_map<consensus::Round,
                         tbb::concurrent_queue<TransactionBatchType>,
                         consensus::RoundTypeHasher>
          current_proposals_;

      /**
       * Read write mutex for public methods
       */
      std::shared_timed_mutex lock_;

      std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
          proposal_factory_;

      /**
       * Processed transactions cache used for replay prevention
       */
      std::shared_ptr<ametsuchi::TxPresenceCache> tx_cache_;

      /**
       * Logger instance
       */
      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
