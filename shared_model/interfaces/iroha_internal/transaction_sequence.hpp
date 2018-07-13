/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_HPP
#define IROHA_TRANSACTION_SEQUENCE_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
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
      TransactionSequence() = delete;

      /**
       * Creator of transaction sequence
       * @param transactions collection of transactions
       * @param validator validator of the collections
       * @return Result containing transaction sequence if validation
       * successful and string message containing error otherwise
       */
      template <typename TransactionValidator, typename OrderValidator>
      static iroha::expected::Result<TransactionSequence, std::string>
      createTransactionSequence(
          const types::SharedTxsCollectionType &transactions,
          const validation::TransactionsCollectionValidator<
              TransactionValidator,
              OrderValidator> &validator);

      /**
       * Get transactions collection
       * @return transactions collection
       */
      types::SharedTxsCollectionType transactions() const;

      /**
       * Get batches in transaction sequence
       * Note that transaction without batch meta are returned as batch with
       * single transaction
       * @return collection of batches from transaction sequence
       */
      types::BatchesType batches() const;

     private:
      explicit TransactionSequence(
          const types::SharedTxsCollectionType &transactions);

      /**
       * Get the concatenation of reduced hashes
       * @param reduced_hashes collection of reduced hashes
       * @return concatenated redueced hashes
       */
      std::string calculateBatchHash(
          std::vector<types::HashType> reduced_hashes) const;

      types::SharedTxsCollectionType transactions_;

      mutable boost::optional<types::BatchesType> batches_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_HPP
