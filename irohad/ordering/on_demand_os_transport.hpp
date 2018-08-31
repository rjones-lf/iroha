/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_OS_TRANSPORT_HPP
#define IROHA_ON_DEMAND_OS_TRANSPORT_HPP

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace shared_model {
  namespace interface {
    class Transaction;
    class Proposal;
    class Peer;
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
      struct Round {
        BlockRoundType block_round;
        RejectRoundType reject_round;

        bool operator<(const Round &rhs) const {
          return std::tie(block_round, reject_round)
              < std::tie(rhs.block_round, rhs.reject_round);
        }

        bool operator==(const Round &rhs) const {
          return std::tie(block_round, reject_round)
              == std::tie(rhs.block_round, rhs.reject_round);
        }
      };

      /**
       * Class provides hash function for Round
       */
      class RoundTypeHasher {
       public:
        std::size_t operator()(const Round &val) const {
          size_t seed = 0;
          boost::hash_combine(seed, val.block_round);
          boost::hash_combine(seed, val.reject_round);
          return seed;
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
            std::shared_ptr<shared_model::interface::Transaction>;

        /**
         * Type of inserted collections
         */
        using CollectionType = std::vector<TransactionType>;

        /**
         * Callback on receiving transactions
         * @param round - expected proposal round
         * @param transactions - vector of passed transactions
         */
        virtual void onTransactions(Round round,
                                    CollectionType transactions) = 0;

        /**
         * Callback on request about proposal
         * @param round - number of collaboration round.
         * Calculated as block_height + 1
         * @return proposal for requested round
         */
        virtual boost::optional<ProposalType> onRequestProposal(
            Round round) = 0;

        virtual ~OdOsNotification() = default;
      };

      /**
       * Factory for creating communication interface to a specific peer
       */
      class OdOsNotificationFactory {
       public:
        /**
         * Create corresponding OdOsNotification interface for peer
         * Returned pointer is guaranteed to be not equal to nullptr
         * @param peer - peer to connect
         * @return connection represented with OdOsNotification interface
         */
        virtual std::unique_ptr<OdOsNotification> create(
            const shared_model::interface::Peer &to) = 0;

        virtual ~OdOsNotificationFactory() = default;
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_OS_TRANSPORT_HPP
