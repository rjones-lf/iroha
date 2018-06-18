/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/transaction_sequence_validator.hpp"

namespace shared_model {
  namespace interface {

    template <typename Validator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::TransactionForwardCollectionType &transactions) {
      Validator validator{};
      auto answer = validator.validate(transactions);
      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionSequence(transactions));
    }

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence<
        validation::TransactionSequenceValidator>(
        const types::TransactionForwardCollectionType &transactions);

    types::TransactionForwardCollectionType
    TransactionSequence::transactions() {
      return transactions_;
    }

    TransactionSequence::TransactionSequence(
        const types::TransactionForwardCollectionType &transactions)
        : transactions_(transactions) {}

  }  // namespace interface
}  // namespace shared_model
