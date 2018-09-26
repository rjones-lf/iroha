/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_UNSAFE_FACTORY_HPP
#define IROHA_TRANSACTION_BATCH_UNSAFE_FACTORY_HPP

#include <memory>

#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Abstract unsafe (no validations are performed) factory for transaction
     * batches
     */
    class TransactionBatchUnsafeFactory {
     public:
      /**
       * Create transaction batch from a single transaction
       * @param transaction to be in the batch
       * @return unique_ptr to the created batch
       */
      std::unique_ptr<TransactionBatch> createTransactionBatch(
          std::unique_ptr<Transaction> transaction);

      /**
       * Create transaction batch from a collection of transactions
       * @param transactions to be in the batch
       * @return unique_ptr to the created batch
       */
      std::unique_ptr<TransactionBatch> createTransactionBatch(
          types::SharedTxsCollectionType transactions);
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_UNSAFE_FACTORY_HPP
