/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_CACHE_HPP
#define IROHA_ON_DEMAND_CACHE_HPP

#include "ordering_gate_cache.hpp"

#include <boost/circular_buffer.hpp>
#include <queue>

namespace iroha {
  namespace ordering {
    namespace cache {

      class OnDemandCache : public OrderingGateCache {
       public:
        void addToBack(const BatchesSetType &batches) override;

        BatchesSetType clearFrontAndGet() override;

        void up() override;

        void remove(const BatchesSetType &batches) override;

        virtual const BatchesSetType &head() const override;

        virtual const BatchesSetType &tail() const override;

       private:
        using BatchesQueueType =
            std::queue<BatchesSetType, boost::circular_buffer<BatchesSetType>>;
        BatchesQueueType queue_{
            boost::circular_buffer<BatchesSetType>(3, BatchesSetType{})};
      };

    }  // namespace cache
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_CACHE_HPP
