/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_factory.hpp"

#include "interfaces/iroha_internal/transaction_batch_template_definitions.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace interface {

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultUnsignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultSignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultUnsignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultSignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

  }  // namespace interface
}  // namespace shared_model
