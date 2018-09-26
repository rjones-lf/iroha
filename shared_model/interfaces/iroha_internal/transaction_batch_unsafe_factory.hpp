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

    class TransactionBatchUnsafeFactory {
      static std::unique_ptr<TransactionBatch> createTransactionBatch(
          std::shared_ptr<Transaction> transaction);

      static std::unique_ptr<TransactionBatch> createTransactionBatch(
          types::SharedTxsCollectionType transactions);
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_UNSAFE_FACTORY_HPP
