/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTIONS_COLLECTION_VALIDATOR_HPP
#define IROHA_TRANSACTIONS_COLLECTION_VALIDATOR_HPP

#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Validator of transaction's collection, this is not fair implementation
     * now, it always returns empty answer
     */
    template <typename TransactionValidator>
    class TransactionsCollectionValidator {
     protected:
      TransactionValidator transaction_validator_;

     private:
      template <typename Validator>
      Answer validateImpl(
          const interface::types::TransactionsForwardCollectionType
              &transactions,
          Validator &&validator) const;

     public:
      explicit TransactionsCollectionValidator(
          const TransactionValidator &transactions_validator =
              TransactionValidator());

      // TODO: IR-1505, igor-egorov, 2018-07-05 Remove method below when
      // proposal and block will return collection of shared transactions
      /**
       * Validates collection of transactions
       * @param transactions collection of transactions
       * @return Answer containing errors if any
       */
      Answer validate(const interface::types::TransactionsForwardCollectionType
                          &transactions) const;

      Answer validate(
          const interface::types::SharedTxsCollectionType &transactions) const;

      Answer validate(const interface::types::TransactionsForwardCollectionType
                          &transactions,
                      interface::types::TimestampType current_timestamp) const;

      Answer validate(
          const interface::types::SharedTxsCollectionType &transactions,
          interface::types::TimestampType current_timestamp) const;

      const TransactionValidator &getTransactionValidator() const;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTIONS_COLLECTION_VALIDATOR_HPP
