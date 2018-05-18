/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTIONS_VALIDATOR_HPP
#define IROHA_TRANSACTIONS_VALIDATOR_HPP

namespace shared_model {
  namespace validation {

    template <typename TransactionValidator>
    class TransactionsValidator {
     protected:
      using TransactionValidatorType = TransactionValidator;

      TransactionsValidator(
          TransactionValidator transaction_validator = TransactionsValidator())
          : transaction_validator_(transaction_validator) {}

      void validateTransactions(
          ReasonsGroupType &reason,
          const interface::types::TransactionsCollectionType &transactions)
          const {
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

#endif  // IROHA_TRANSACTIONS_VALIDATOR_HPP
