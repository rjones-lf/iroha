/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_OG_CACHE_HPP
#define IROHA_OG_CACHE_HPP

#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace iroha {
  namespace ordering {
    namespace cache {

      class OgCache {
       public:
        using BatchesListType = std::set<
            std::shared_ptr<shared_model::interface::TransactionBatch>>;

        virtual void addToBack(
            std::shared_ptr<shared_model::interface::TransactionBatch>
                batch) = 0;
        virtual BatchesListType dequeue() = 0;
        virtual void remove(const BatchesListType &batches) = 0;
      };

    }  // namespace cache

  }  // namespace ordering
}  // namespace iroha

#endif  // IROHA_OG_CACHE_HPP
