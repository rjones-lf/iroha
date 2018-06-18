/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/transaction_sequence_validator.hpp"

namespace shared_model {
  namespace interface {

    TransactionSequence::TransactionSequence(
        const boost::any_range<Transaction, boost::forward_traversal_tag>
            &transactions)
        : transactions_(transactions) {}

    template <typename Validator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::TransactionsCollectionType &transactions) {
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
        const types::TransactionsCollectionType &transactions);

  }  // namespace interface
}  // namespace shared_model
