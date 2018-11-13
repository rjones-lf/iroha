/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_CACHE_HPP
#define IROHA_ON_DEMAND_CACHE_HPP

#include "ordering/impl/ordering_gate_cache/ordering_gate_cache.hpp"

#include <boost/circular_buffer.hpp>
#include <queue>
#include <shared_mutex>

namespace iroha {
  namespace ordering {
    namespace cache {

      class OnDemandCache : public OrderingGateCache {
       public:
        void addToBack(const BatchesSetType &batches) override;

        BatchesSetType pop() override;

        void remove(const BatchesSetType &batches) override;

        virtual const BatchesSetType &head() const override;

        virtual const BatchesSetType &tail() const override;

       private:
        mutable std::shared_timed_mutex mutex_;
        using BatchesQueueType = boost::circular_buffer<BatchesSetType>;
        BatchesQueueType circ_buffer{3, BatchesSetType{}};
      };

    }  // namespace cache
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_CACHE_HPP
