/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
#define IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP

#include "validators/answer.hpp"
#include "validators/field_validator.hpp"
#include "validators/transactions_collection/any_order_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Validator of transaction's collection, this is not fair implementation
     * now, it always returns empty answer
     */
    template <typename TransactionValidator,
              typename FieldValidator = validation::FieldValidator>
    class TransactionsCollectionValidator {
     protected:
      TransactionValidator transaction_validator_;
      FieldValidator field_validator_;

     public:
      explicit TransactionsCollectionValidator(
          const TransactionValidator &transactions_validator =
              TransactionValidator(),
          const FieldValidator &field_validator = FieldValidator());

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

      const TransactionValidator &getTransactionValidator() const;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
