/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pending_txs_storage/pending_txs_storage.hpp"

#include "multi_sig_transactions/state/mst_state.hpp"
#include "pending_txs_storage.hpp"
#include "pending_txs_storage/pending_txs_storage.hpp"

namespace iroha {

  PendingTransactionStorage::PendingTransactionStorage(
      StateObservable updatedBatches,
      BatchObservable preparedBatch,
      BatchObservable expiredBatch) {
    updated_batches_subscription_ =
        updatedBatches.subscribe([this](const SharedState &updatedBatches) {
          this->updatedBatchesHandler(updatedBatches);
        });
    prepared_batch_subscription_ =
        preparedBatch.subscribe([this](const SharedBatch &preparedBatch) {
          this->preparedBatchHandler(preparedBatch);
        });
    expired_batch_subscription_ =
        expiredBatch.subscribe([this](const SharedBatch &expiredBatch) {
          this->expiredBatchHandler(expiredBatch);
        });
  }

  PendingTransactionStorage::~PendingTransactionStorage() {
    updated_batches_subscription_.unsubscribe();
    prepared_batch_subscription_.unsubscribe();
    expired_batch_subscription_.unsubscribe();
  }

  PendingTransactionStorage::SharedTxsCollectionType
  PendingTransactionStorage::getPendingTransactions(
      const AccountIdType &accountId) const {
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    auto creator_it = storage_.index.find(accountId);
    if (storage_.index.end() != creator_it) {
      auto &batch_hashes = creator_it->second;
      SharedTxsCollectionType result;
      auto &batches = storage_.batches;
      for (const auto &batch_hash : batch_hashes) {
        auto batch_it = batches.find(batch_hash);
        if (batches.end() != batch_it) {
          auto &txs = batch_it->second->transactions();
          result.insert(result.end(), txs.begin(), txs.end());
        }
      }
      return result;
    }
    return {};
  }

  std::set<PendingTransactionStorage::AccountIdType>
  PendingTransactionStorage::batchCreators(
      const TransactionBatch &batch) const {
    std::set<AccountIdType> creators;
    for (const auto &transaction : batch.transactions()) {
      creators.insert(transaction->creatorAccountId());
    }
    return creators;
  }

  void PendingTransactionStorage::updatedBatchesHandler(
      const SharedState &updatedBatches) {
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    for (auto &batch : updatedBatches->getBatches()) {
      auto hash = batch->reducedHash();
      auto it = storage_.batches.find(hash);
      if (storage_.batches.end() == it) {
        for (const auto &creator : batchCreators(*batch)) {
          storage_.index[creator].insert(hash);
        }
      }
      storage_.batches[hash] = batch;
    }
  }

  void PendingTransactionStorage::preparedBatchHandler(
      const SharedBatch &preparedBatch) {
    removeBatch(preparedBatch);
  }

  void PendingTransactionStorage::expiredBatchHandler(
      const SharedBatch &expiredBatch) {
    removeBatch(expiredBatch);
  }

  void PendingTransactionStorage::removeBatch(const SharedBatch &batch) {
    auto creators = batchCreators(*batch);
    auto hash = batch->reducedHash();
    {
      std::unique_lock<std::shared_timed_mutex> lock(mutex_);
      auto &batches = storage_.batches;
      auto batch_it = batches.find(hash);
      if (batches.end() != batch_it) {
        batches.erase(batch_it);
      }
      for (const auto &creator : creators) {
        auto &index = storage_.index;
        auto index_it = index.find(creator);
        if (index.end() != index_it) {
          auto &creator_set = index_it->second;
          auto creator_it = creator_set.find(hash);
          if (creator_set.end() != creator_it) {
            creator_set.erase(creator_it);
          }
        }
      }
    }
  }

}  // namespace iroha
