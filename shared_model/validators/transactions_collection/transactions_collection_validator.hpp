/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
#define IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP

#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "validators/answer.hpp"
#include "validators/transactions_collection/any_order_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Validator of transaction's collection, this is not fair implementation
     * now, it always returns empty answer
     */
    template <typename TransactionValidator,
              typename OrderValidator = AnyOrderValidator>
    class TransactionsCollectionValidator {
     protected:
      TransactionValidator transaction_validator_;
      OrderValidator order_validator_;

     public:
      TransactionsCollectionValidator(
          const TransactionValidator &transactions_validator =
              TransactionValidator(),
          const OrderValidator &order_validator = OrderValidator())
          : order_validator_(order_validator) {}

      /**
       * Validates collection of transactions
       * @param transactions collection of transactions
       * @return Answer containing errors if any
       */
      virtual Answer validate(
          const interface::types::TransactionsForwardCollectionType
              &transactions) const = 0;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
