/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_PRESENCE_CACHE_IMPL_HPP
#define IROHA_TX_PRESENCE_CACHE_IMPL_HPP

#include "ametsuchi/tx_presence_cache.hpp"
#include "cache/cache.hpp"

namespace iroha {
  namespace ametsuchi {

    class TxPresenceCacheImpl : public TxPresenceCache {
     public:
      TxCacheStatusType check(
          const shared_model::crypto::Hash &hash) const override;

      BatchStatusCollectionType check(
          const shared_model::interface::TransactionBatch &batch)
          const override;

     private:
      mutable cache::Cache<shared_model::crypto::Hash,
                           TxCacheStatusType,
                           shared_model::crypto::Hash::Hasher>
          memory_cache_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TX_PRESENCE_CACHE_IMPL_HPP
