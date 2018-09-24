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

#include "logger/logger.hpp"

namespace iroha {
  namespace ordering {
    class OnDemandOrderingServiceImpl : public OnDemandOrderingService {
     public:
      /**
       * Create on_demand ordering service with following options:
       * @param transaction_limit - number of maximum transactions in a one
       * proposal
       * @param number_of_proposals - number of stored proposals, older will be
       * removed. Default value is 3
       * @param initial_round - first round of agreement.
       * Default value is {2, 1} since genesis block height is 1
       */
      explicit OnDemandOrderingServiceImpl(
          size_t transaction_limit,
          size_t number_of_proposals,
          const transport::Round &initial_round);

      explicit OnDemandOrderingServiceImpl(size_t transaction_limit)
          : OnDemandOrderingServiceImpl(transaction_limit, 3, {2, 1}) {}

      // --------------------- | OnDemandOrderingService |_---------------------

      void onCollaborationOutcome(transport::Round round) override;

      // ----------------------- | OdOsNotification | --------------------------

      void onTransactions(transport::Round,
                          CollectionType transactions) override;

      boost::optional<ProposalType> onRequestProposal(
          transport::Round round) override;

     private:
      /**
       * Packs new proposals and creates new rounds
       * Note: method is not thread-safe
       */
      void packNextProposals(const transport::Round &round);

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
      ProposalType emitProposal(const transport::Round &round);

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
      std::queue<transport::Round> round_queue_;

      /**
       * Map of available proposals
       */
      std::unordered_map<transport::Round,
                         ProposalType,
                         transport::RoundTypeHasher>
          proposal_map_;

      /**
       * Proposals for current rounds
       */
      std::unordered_map<transport::Round,
                         tbb::concurrent_queue<TransactionType>,
                         transport::RoundTypeHasher>
          current_proposals_;

      /**
       * Read write mutex for public methods
       */
      std::shared_timed_mutex lock_;

      /**
       * Logger instance
       */
      logger::Logger log_;
    };
  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_SERVICE_IMPL_HPP
