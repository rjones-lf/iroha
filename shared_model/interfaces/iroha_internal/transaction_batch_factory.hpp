/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_FACTORY_HPP
#define IROHA_TRANSACTION_BATCH_FACTORY_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace interface {

    class TransactionBatchFactory {
     public:
      /**
       * Create transaction batch out of collection of transactions
       * @tparam TransactionValidator validates every single transaction
       * @tparam OrderValidator validates order of transactions
       * @param transactions collection of transactions, should be from the same
       * batch
       * @param validator transactions collection validator with provided
       * transaction validator and order validator
       * @return valid batch of transactions
       */
      template <typename TransactionValidator,
                typename FieldValidator = validation::FieldValidator>
      static iroha::expected::Result<TransactionBatch, std::string>
      createTransactionBatch(const types::SharedTxsCollectionType &transactions,
                             const validation::TransactionsCollectionValidator<
                                 TransactionValidator> &validator,
                             const FieldValidator & = FieldValidator());

      /**
       * Creates transaction batch from single transaction
       * @tparam TransactionValidator validates every single transaction
       * @param transaction is transaction being validated and used to create
       * batch
       * @param transaction_validator transaction validation logic
       * @return batch with single transaction
       * @note transactions in such batches may not have batch meta information
       */
      template <typename TransactionValidator,
                typename FieldValidator = validation::FieldValidator>
      static iroha::expected::Result<TransactionBatch, std::string>
      createTransactionBatch(
          std::shared_ptr<Transaction> transaction,
          const TransactionValidator &transaction_validator =
              TransactionValidator(),
          const FieldValidator &field_validator = FieldValidator());
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_FACTORY_HPP
