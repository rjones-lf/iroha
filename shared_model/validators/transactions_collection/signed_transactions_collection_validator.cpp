/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/transactions_collection/signed_transactions_collection_validator.hpp"

#include <boost/format.hpp>
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename TransactionValidator>
    Answer
    SignedTransactionsCollectionValidator<TransactionValidator>::validate(
        const interface::types::TransactionsForwardCollectionType &transactions)
        const {
      ReasonsGroupType reason;
      reason.first = "Transaction list";
      for (const auto &tx : transactions) {
        auto answer =
            SignedTransactionsCollectionValidator::transaction_validator_
                .validate(tx);
        if (answer.hasErrors()) {
          auto message =
              (boost::format("Tx %s : %s") % tx.hash().hex() % answer.reason())
                  .str();
          reason.second.push_back(message);
        }
      }

      Answer res;
      if (not reason.second.empty()) {
        res.addReason(std::move(reason));
      }
      return res;
    }

    template Answer SignedTransactionsCollectionValidator<
        TransactionValidator<FieldValidator,
                             CommandValidatorVisitor<FieldValidator>>>::
        validate(const interface::types::TransactionsForwardCollectionType
                     &transactions) const;

  }  // namespace validation
}  // namespace shared_model
