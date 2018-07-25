/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PENDING_TXS_STORAGE_HPP
#define IROHA_PENDING_TXS_STORAGE_HPP

#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include <rxcpp/rx.hpp>
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace iroha {

  class MstState;

  class PendingTransactionStorage {
   public:
    using AccountIdType = shared_model::interface::types::AccountIdType;
    using HashType = shared_model::interface::types::HashType;
    using SharedTxsCollectionType =
        shared_model::interface::types::SharedTxsCollectionType;
    using TransactionBatch = shared_model::interface::TransactionBatch;
    using SharedState = std::shared_ptr<MstState>;
    using SharedBatch = std::shared_ptr<TransactionBatch>;
    using StateObservable = rxcpp::observable<SharedState>;
    using BatchObservable = rxcpp::observable<SharedBatch>;

    PendingTransactionStorage(StateObservable updatedBatches,
                              BatchObservable preparedBatch,
                              BatchObservable expiredBatch);

    ~PendingTransactionStorage();

    SharedTxsCollectionType getPendingTransactions(
        const AccountIdType &accountId) const;

   private:
    void updatedBatchesHandler(const SharedState &updatedBatches);
    void preparedBatchHandler(const SharedBatch &preparedBatch);
    void expiredBatchHandler(const SharedBatch &expiredBatch);

    void removeBatch(const SharedBatch &batch);

    std::set<AccountIdType> batchCreators(const TransactionBatch &batch) const;

    /**
     * Subscriptions on MST events
     */
    rxcpp::composite_subscription updated_batches_subscription_;
    rxcpp::composite_subscription prepared_batch_subscription_;
    rxcpp::composite_subscription expired_batch_subscription_;

    /**
     * Mutex for single-write multiple-read storage access
     */
    mutable std::shared_timed_mutex mutex_;

    struct {
      std::unordered_map<AccountIdType,
                         std::unordered_set<HashType, HashType::Hasher>>
          index;
      std::unordered_map<HashType,
                         std::shared_ptr<TransactionBatch>,
                         HashType::Hasher>
          batches;
    } storage_;
  };

}  // namespace iroha

#endif  // IROHA_PENDING_TXS_STORAGE_HPP
