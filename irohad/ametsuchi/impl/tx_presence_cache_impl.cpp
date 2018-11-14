/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/tx_presence_cache_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"

namespace iroha {
  namespace ametsuchi {
    TxCacheStatusType TxPresenceCacheImpl::check(
        const shared_model::crypto::Hash &hash) const {
      auto res = memory_cache_.findItem(hash);
      if (res != boost::none) {
        return res.get();
      }
      // TODO: do storage request
      // if hash is in a storage, put it to the cache
      return tx_cache_status_responses::Missing();
    }

    TxPresenceCache::BatchStatusCollectionType TxPresenceCacheImpl::check(
        const shared_model::interface::TransactionBatch &batch) const {
      TxPresenceCache::BatchStatusCollectionType batch_statuses{};
      for (const auto &tx : batch.transactions()) {
        batch_statuses.emplace_back(check(tx->hash()));
      }
      return batch_statuses;
    }
  }  // namespace ametsuchi
}  // namespace iroha
