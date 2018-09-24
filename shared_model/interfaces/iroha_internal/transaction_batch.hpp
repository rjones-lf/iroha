/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_HPP
#define IROHA_TRANSACTION_BATCH_HPP

#include "interfaces/common_objects/transaction_sequence_common.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Represents collection of transactions, which are to be processed together
     */
    class TransactionBatch {
     public:
      virtual ~TransactionBatch() = default;

      /**
       * Get transactions list
       * @return list of transactions from the batch
       */
      virtual const types::SharedTxsCollectionType &transactions() const = 0;

      /**
       * Get the concatenation of reduced hashes as a single hash
       * @param reduced_hashes collection of reduced hashes
       * @return concatenated reduced hashes
       */
      virtual const types::HashType &reducedHash() const = 0;

      /**
       * Checks if every transaction has quorum signatures
       * @return true if every transaction has quorum signatures, false
       * otherwise
       */
      virtual bool hasAllSignatures() const = 0;

      /**
       * Checks of two transaction batches are the same
       * @param rhs - another batch
       * @return true if they are equal, false otherwise
       */
      virtual bool operator==(const TransactionBatch &rhs) const = 0;

      /**
       * @return string representation of the object
       */
      virtual std::string toString() const = 0;

      /**
       * Add signature to concrete transaction in the batch
       * @param number_of_tx - number of transaction for inserting signature
       * @param signed_blob - signed blob of transaction
       * @param public_key - public key of inserter
       * @return true if signature has been inserted
       */
      virtual bool addSignature(
          size_t number_of_tx,
          const shared_model::crypto::Signed &signed_blob,
          const shared_model::crypto::PublicKey &public_key) = 0;

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
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_HPP
