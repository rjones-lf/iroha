/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_PRESENCE_CACHE_HPP
#define IROHA_TX_PRESENCE_CACHE_HPP

#include <vector>

#include "ametsuchi/tx_cache_outcome.hpp"

namespace shared_model {
  namespace crypto {
    class Hash;
  }  // namespace crypto

  namespace interface {
    class TransactionBatch;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    /**
     * Class is responsible for checking transaction status in the ledger
     */
    class TxPresenceCache {
     public:
      /**
       * Check transaction status by hash
       * @param hash - corresponding hash of the interested transaction
       */
      virtual TxCacheOutcome check(
          const shared_model::crypto::Hash &hash) const = 0;

      /**
       * Check batch status
       * @param batch - interested batch of transactions
       * @return a collection with answers about each transaction in the batch
       */
      virtual std::vector<TxCacheOutcome> check(
          const shared_model::interface::TransactionBatch &batch) const = 0;

      // TODO: 09/11/2018 @muratovv add method for processing collection of
      // batches IR-1857

      virtual ~TxPresenceCache() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_TX_PRESENCE_CACHE_HPP
