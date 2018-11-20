/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/tx_presence_cache_impl.hpp"
#include "common/visitor.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"
#include "tx_presence_cache_impl.hpp"

namespace iroha {
  namespace ametsuchi {
    TxPresenceCacheImpl::TxPresenceCacheImpl(std::shared_ptr<Storage> storage)
        : storage_(storage) {}

    TxCacheStatusType TxPresenceCacheImpl::check(
        const shared_model::crypto::Hash &hash) const {
      auto res = memory_cache_.findItem(hash);
      if (res != boost::none) {
        return res.get();
      }
      return checkInStorage(hash);
    }

    TxPresenceCache::BatchStatusCollectionType TxPresenceCacheImpl::check(
        const shared_model::interface::TransactionBatch &batch) const {
      TxPresenceCache::BatchStatusCollectionType batch_statuses;
      for (const auto &tx : batch.transactions()) {
        batch_statuses.emplace_back(check(tx->hash()));
      }
      return batch_statuses;
    }

    TxCacheStatusType TxPresenceCacheImpl::checkInStorage(
        const shared_model::crypto::Hash &hash) const {
      TxCacheStatusType cache_status_check;
      visit_in_place(
          storage_->getBlockQuery()->checkTxPresence(hash),
          [&](const tx_cache_status_responses::Committed &status) {
            memory_cache_.addItem(hash, status);
            cache_status_check = tx_cache_status_responses::Committed(hash);
          },
          [&](const tx_cache_status_responses::Rejected &status) {
            memory_cache_.addItem(hash, status);
            cache_status_check = tx_cache_status_responses::Rejected(hash);
          },
          [&](const tx_cache_status_responses::Missing &) {
            // don't put this hash into cache since "Missing" can become
            // "Committed" or "Rejected" later
            cache_status_check = tx_cache_status_responses::Missing(hash);
          });

      return cache_status_check;
    }
  }  // namespace ametsuchi
}  // namespace iroha
