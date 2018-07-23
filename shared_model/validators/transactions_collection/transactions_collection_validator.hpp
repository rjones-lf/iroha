/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
#define IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP

#include <algorithm>
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

      // TODO: IR-1505, igor-egorov, 2018-07-05 Remove method below when
      // proposal and block will return collection of shared transactions
      /**
       * Validates collection of transactions
       * @param transactions collection of transactions
       * @return Answer containing errors if any
       */
      Answer validate(const interface::types::TransactionsForwardCollectionType
                          &transactions) const {
        interface::types::SharedTxsCollectionType res;
        std::transform(std::begin(transactions),
                       std::end(transactions),
                       std::back_inserter(res),
                       [](const auto &tx) { return clone(tx); });
        return validatePointers(res);
      }

      const TransactionValidator &getTransactionValidator() const {
        return transaction_validator_;
      }

      virtual Answer validatePointers(
          const interface::types::SharedTxsCollectionType &transactions)
          const = 0;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
