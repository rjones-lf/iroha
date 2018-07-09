/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_OS_TRANSPORT_HPP
#define IROHA_ON_DEMAND_OS_TRANSPORT_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace shared_model {
  namespace interface {
    class Transaction;
    class Proposal;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ordering {
    namespace transport {

      /**
       * Type of round indexing by blocks
       */
      using BlockRoundType = uint64_t;

      /**
       * Type of round indexing by reject before new block commit
       */
      using RejectRoundType = uint32_t;

      /**
       * Type of proposal round
       */
      using RoundType = std::pair<BlockRoundType, RejectRoundType>;

      /**
       * Class provides hash function for RoundType
       */
      class RoundTypeHasher {
       public:
        std::size_t operator()(const RoundType &val) const {
          return boost::hash_value(val);
        }
      };

      /**
       * Notification interface of on demand ordering service.
       */
      class OdOsNotification {
       public:
        /**
         * Type of stored proposals
         */
        using ProposalType = std::unique_ptr<shared_model::interface::Proposal>;

        /**
         * Type of stored transactions
         */
        using TransactionType =
            std::unique_ptr<shared_model::interface::Transaction>;

        /**
         * Type of inserted collections
         */
        using CollectionType = std::vector<TransactionType>;

        /**
         * Callback on receiving transactions
         * @param transactions - vector of passed transactions
         */
        virtual void onTransactions(CollectionType &&transactions) = 0;

        /**
         * Callback on request about proposal
         * @param round - number of collaboration round.
         * Calculated as block_height + 1
         * @return proposal for requested round
         */
        virtual boost::optional<ProposalType> onRequestProposal(
            RoundType round) = 0;

        virtual ~OdOsNotification() = default;
      };
    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha
#endif  // IROHA_ON_DEMAND_OS_TRANSPORT_HPP
