/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_OS_TRANSPORT_HPP
#define IROHA_ON_DEMAND_OS_TRANSPORT_HPP

#include <boost/optional.hpp>
#include <cstdint>
#include <memory>
#include <vector>

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
       * Type of round indexing
       */
      using RoundType = uint64_t;

      /**
       * Notification interface of on demand ordering service.
       */
      class OdOsNotification {
       public:
        /**
         * Callback on receiving transactions
         * @param transactions - vector of passed transactions
         */
        virtual void onTransactions(
            std::vector<std::shared_ptr<shared_model::interface::Transaction>>
                transactions) = 0;

        /**
         * Callback on request about proposal
         * @param round - number of collaboration round.
         * Calculated as block_height + 1
         * @return proposal for requested round
         */
        virtual boost::optional<
            std::shared_ptr<shared_model::interface::Proposal>>
        onRequestProposal(RoundType round) = 0;

        virtual ~OdOsNotification() = default;
      };
    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha
#endif  // IROHA_ON_DEMAND_OS_TRANSPORT_HPP
