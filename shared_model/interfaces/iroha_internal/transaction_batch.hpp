/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_HPP
#define IROHA_TRANSACTION_BATCH_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace interface {

    class TransactionBatch {
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
      template <typename TransactionValidator, typename OrderValidator>
      static iroha::expected::Result<TransactionBatch, std::string>
      createTransactionBatch(const types::SharedTxsCollectionType &transactions,
                             const validation::TransactionsCollectionValidator<
                                 TransactionValidator,
                                 OrderValidator> &validator);

      /**
       * Creates transaction batch from single transaction
       * @tparam TransactionValidator validates every single transaction
       * @param transaction is transaction being validated and used to create
       * batch
       * @param transaction_validator transaction validation logic
       * @return batch with single transaction
       * @note transactions in such batches may not have batch meta information
       */
      template <typename TransactionValidator>
      static iroha::expected::Result<TransactionBatch, std::string>
      createTransactionBatch(std::shared_ptr<Transaction> transaction,
                             const TransactionValidator &transaction_validator =
                                 TransactionValidator());

      /**
       * Get transactions list
       * @return list of transactions from the batch
       */
      const types::SharedTxsCollectionType &transactions() const;

      /**
       * Get the concatenation of reduced hashes as a single hash
       * @param reduced_hashes collection of reduced hashes
       * @return concatenated reduced hashes
       */
      const types::HashType &reducedHash() const;

      /**
       * Checks if every transaction has quorum signatures
       * @return true if every transaction has quorum signatures, false
       * otherwise
       */
      bool hasAllSignatures() const;

      /**
       * Get the concatenation of reduced hashes as a single hash
       * That kind of hash does not respect batch type
       * @param reduced_hashes
       * @return concatenated reduced hashes
       */
      static types::HashType calculateReducedBatchHash(
          const boost::any_range<types::HashType, boost::forward_traversal_tag>
              &reduced_hashes);

     private:
      explicit TransactionBatch(
          const types::SharedTxsCollectionType &transactions)
          : transactions_(transactions) {}

      types::SharedTxsCollectionType transactions_;

      mutable boost::optional<types::HashType> reduced_hash_;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_HPP
