/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_HPP
#define IROHA_TRANSACTION_BATCH_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "validators/field_validator.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace interface {

    class TransactionBatch {
     public:
      TransactionBatch() = delete;
      TransactionBatch(const TransactionBatch &) = default;
      TransactionBatch(TransactionBatch &&) = default;

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

      explicit TransactionBatch(
          const types::SharedTxsCollectionType &transactions)
          : transactions_(transactions) {}

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

      bool operator==(const TransactionBatch &rhs) const;

      /**
       * @return string representation of the object
       */
      std::string toString() const;

      /**
       * Add signature to concrete transaction in the batch
       * @param number_of_tx - number of transaction for inserting signature
       * @param singed - signed blob of transaction
       * @param public_key - public key of inserter
       * @return true if signature has been inserted
       */
      bool addSignature(size_t number_of_tx,
                        const shared_model::crypto::Signed &signed_blob,
                        const shared_model::crypto::PublicKey &public_key);

      /**
       * Get the concatenation of reduced hashes as a single hash
       * That kind of hash does not respect batch type
       * @tparam Collection type of const ref iterator
       * @param reduced_hashes
       * @return concatenated reduced hashes
       */
      template <typename Collection>
      static types::HashType calculateReducedBatchHash(
          const Collection &reduced_hashes) {
        std::stringstream concatenated_hash;
        for (const auto &hash : reduced_hashes) {
          concatenated_hash << hash.hex();
        }
        return types::HashType::fromHexString(concatenated_hash.str());
      }

     private:
      types::SharedTxsCollectionType transactions_;

      mutable boost::optional<types::HashType> reduced_hash_;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_HPP
