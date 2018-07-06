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
      /**
       * Creator of transaction sequence
       * @param transactions collection of transactions
       * @param validator validator of the collections
       * @return Result containing transaction sequence if validation successful
       * and string message containing error otherwise
       */
      template <typename TransactionValidator>
      static iroha::expected::Result<TransactionSequence, std::string>
      createTransactionSequence(
          const types::TransactionsForwardCollectionType &transactions,
          const validation::TransactionsCollectionValidator<
              TransactionValidator> &validator);

      /**
       * Get transactions collection
       * @return transactions collection
       */
      types::TransactionsForwardCollectionType transactions();

     private:
      explicit TransactionSequence(
          const types::TransactionsForwardCollectionType &transactions);

      types::TransactionsForwardCollectionType transactions_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_HPP
