/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_NON_EMPTY_TRANSACTIONS_VALIDATOR_HPP
#define IROHA_NON_EMPTY_TRANSACTIONS_VALIDATOR_HPP

#include <boost/format.hpp>

#include "interfaces/common_objects/types.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    template <typename TransactionValidator>
    class NonEmptyTransactionsValidator {
     protected:
      using TransactionValidatorType = TransactionValidator;

      NonEmptyTransactionsValidator(
          TransactionValidator transaction_validator = TransactionValidator())
          : transaction_validator_(transaction_validator) {}

      void validateTransactions(
          ReasonsGroupType &reason,
          const interface::types::TransactionsCollectionType &transactions)
          const {
        if (transactions.empty()) {
          reason.second.push_back("Transactions can not be empty");
          return;
        }
        for (const auto &tx : transactions) {
          validateTransaction(reason, *tx);
        }
      }

     private:
      void validateTransaction(
          ReasonsGroupType &reason,
          const interface::Transaction &transaction) const {
        auto answer = transaction_validator_.validate(transaction);
        if (answer.hasErrors()) {
          auto message = (boost::format("Tx: %s") % answer.reason()).str();
          reason.second.push_back(message);
        }
      }

      TransactionValidator transaction_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_NON_EMPTY_TRANSACTIONS_VALIDATOR_HPP
