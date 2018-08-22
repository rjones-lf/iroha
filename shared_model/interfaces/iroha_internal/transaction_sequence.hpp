/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_HPP
#define IROHA_TRANSACTION_SEQUENCE_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Transaction sequence is the collection of transactions where:
     * 1. All transactions from the same batch are place contiguously
     * 2. All batches are full (no transaction from the batch can be outside
     * sequence)
     */
    class TransactionSequence {
     public:
      /**
       * Creator of transaction sequence
       * @param transactions collection of transactions
       * @param validator validator of the collections
       * @return Result containing transaction sequence if validation
       * successful and string message containing error otherwise
       */
      template <typename TransactionValidator,
                typename FieldValidator = validation::FieldValidator>
      static iroha::expected::Result<TransactionSequence, std::string>
      createTransactionSequence(
          const types::SharedTxsCollectionType &transactions,
          const validation::TransactionsCollectionValidator<
              TransactionValidator> &validator,
          const FieldValidator &field_validator = FieldValidator());

      /**
       * Retrieves transactions from all batches as single collection
       * @return all batches transactions
       */
      const types::SharedTxsCollectionType &transactions() const;

      /**
       * Get batches in transaction sequence
       * Note that transaction without batch meta are returned as batch with
       * single transaction
       * @return collection of batches from transaction sequence
       */
      const types::BatchesCollectionType &batches() const;

      std::string toString() const;

     private:
      explicit TransactionSequence(const types::BatchesCollectionType &batches);

      types::BatchesCollectionType batches_;
      mutable boost::optional<types::SharedTxsCollectionType> transactions_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_HPP
