/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/transactions_collection/transactions_collection_validator.hpp"

#include <algorithm>
#include <boost/format.hpp>

#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "validators/default_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/signable_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename TransactionValidator, typename FieldValidator>
    TransactionsCollectionValidator<TransactionValidator, FieldValidator>::
        TransactionsCollectionValidator(
            const TransactionValidator &transactions_validator,
            const FieldValidator &field_validator)
        : transaction_validator_(transactions_validator),
          field_validator_(field_validator) {}

    template <typename TransactionValidator, typename FieldValidator>
    Answer
    TransactionsCollectionValidator<TransactionValidator, FieldValidator>::
        validate(const shared_model::interface::types::
                     TransactionsForwardCollectionType &transactions) const {
      interface::types::SharedTxsCollectionType res;
      std::transform(std::begin(transactions),
                     std::end(transactions),
                     std::back_inserter(res),
                     [](const auto &tx) { return clone(tx); });
      return validate(res);
    }

    template <typename TransactionValidator, typename FieldValidator>
    Answer
    TransactionsCollectionValidator<TransactionValidator, FieldValidator>::
        validate(const shared_model::interface::types::SharedTxsCollectionType
                     &transactions) const {
      Answer res;
      ReasonsGroupType reason;
      reason.first = "Transaction list";

      if (boost::empty(transactions)) {
        reason.second.emplace_back("Transaction sequence can not be empty");
        res.addReason(std::move(reason));
        return res;
      }

      for (const auto &tx : transactions) {
        auto answer = transaction_validator_.validate(*tx);
        if (answer.hasErrors()) {
          auto message =
              (boost::format("Tx %s : %s") % tx->hash().hex() % answer.reason())
                  .str();
          reason.second.push_back(message);
        }
      }

      if (not reason.second.empty()) {
        res.addReason(std::move(reason));
      }
      return res;
    }

    template <typename TransactionValidator, typename FieldValidator>
    const TransactionValidator &TransactionsCollectionValidator<
        TransactionValidator,
        FieldValidator>::getTransactionValidator() const {
      return transaction_validator_;
    }

    template class TransactionsCollectionValidator<DefaultUnsignedTransactionValidator,
                                                   FieldValidator>;

    template class TransactionsCollectionValidator<
        DefaultSignedTransactionValidator,
        FieldValidator>;

  }  // namespace validation
}  // namespace shared_model
