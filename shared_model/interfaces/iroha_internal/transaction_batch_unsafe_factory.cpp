/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_unsafe_factory.hpp"

#include "interfaces/iroha_internal/transaction_batch_impl.hpp"

std::unique_ptr<shared_model::interface::TransactionBatch>
shared_model::interface::TransactionBatchUnsafeFactory::createTransactionBatch(
    std::shared_ptr<shared_model::interface::Transaction> transaction) {
  return std::make_unique<TransactionBatchImpl>(
      shared_model::interface::types::SharedTxsCollectionType{
          std::move(transaction)});
}

std::unique_ptr<shared_model::interface::TransactionBatch>
shared_model::interface::TransactionBatchUnsafeFactory::createTransactionBatch(
    shared_model::interface::types::SharedTxsCollectionType transactions) {
  return std::make_unique<TransactionBatchImpl>(std::move(transactions));
}
