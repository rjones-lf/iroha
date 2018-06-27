/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace interface {

    template <typename TransactionValidator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::TransactionsForwardCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator>
            &validator) {
      auto answer = validator.validate(transactions);
      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionSequence(transactions));
    }

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::TransactionsForwardCollectionType &transactions,
        const validation::TransactionsCollectionValidator<
            validation::TransactionValidator<
                validation::FieldValidator,
                validation::CommandValidatorVisitor<
                    validation::FieldValidator>>> &validator);

    types::TransactionsForwardCollectionType
    TransactionSequence::transactions() {
      return transactions_;
    }

    TransactionSequence::TransactionSequence(
        const types::TransactionsForwardCollectionType &transactions)
        : transactions_(transactions) {}

  }  // namespace interface
}  // namespace shared_model
