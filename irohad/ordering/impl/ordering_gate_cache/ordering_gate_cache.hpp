/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_ORDERING_CACHE_HPP
#define IROHA_ON_DEMAND_ORDERING_CACHE_HPP

#include <unordered_set>

#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace iroha {
  namespace ordering {
    namespace cache {

      /**
       * Cache for transactions sent to ordering gate
       */
      class OrderingGateCache {
       private:
        /**
         * Hasher for the shared pointer on the batch. Uses batch's reduced hash
         */
        struct BatchPointerHasher {
          shared_model::crypto::Hash::Hasher hasher_;

          size_t operator()(
              const std::shared_ptr<shared_model::interface::TransactionBatch>
                  &a) const;
        };

       public:
        using BatchesSetType = std::unordered_set<
            std::shared_ptr<shared_model::interface::TransactionBatch>,
            BatchPointerHasher>;

        /**
         * Concatenates batches from the tail of the queue with provided batches
         * @param batches set of batches that are added to the queue
         */
        virtual void addToBack(const BatchesSetType &batches) = 0;

        /**
         * Shifts all batches from the queue towards the head. Batches from the
         * middle of the queue are concatenated with the head batches. Example:
         *
         * HEAD:    {a,b,c}      {a,b,c,d,e}
         * MIDDLE:  {d,e}  up=>  {f,g}
         * TAIL:    {f,g}        {}
         */
        virtual void up() = 0;

        /**
         * Return all batches from the head and clean the head. Example:
         *
         * HEAD:    {a,b,c}                     {}
         * MIDDLE:  {d,e}   clearFrontAndGet => {d,e}
         * TAIL:    {f,g}                       {f,g}
         * result == {a,b,c}
         *
         */
        virtual BatchesSetType clearFrontAndGet() = 0;

        /**
         * Remove provided batches from the head of the queue
         * @param batches are the batches that are removed from the head of the
         * queue
         */
        virtual void remove(const BatchesSetType &batches) = 0;

        /**
         * Return the head batches
         */
        virtual const BatchesSetType &head() const = 0;

        /**
         * Return the tail batches
         */
        virtual const BatchesSetType &tail() const = 0;

        virtual ~OrderingGateCache() = default;
      };

    }  // namespace cache

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_ORDERING_CACHE_HPP
