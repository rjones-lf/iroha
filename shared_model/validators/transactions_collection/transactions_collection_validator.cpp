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

    template <typename TransactionValidator>
    TransactionsCollectionValidator<TransactionValidator>::
        TransactionsCollectionValidator(
            const TransactionValidator &transactions_validator)
        : transaction_validator_(transactions_validator) {}

    template <typename TransactionValidator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const shared_model::interface::types::TransactionsForwardCollectionType
            &transactions) const {
      interface::types::SharedTxsCollectionType res;
      std::transform(std::begin(transactions),
                     std::end(transactions),
                     std::back_inserter(res),
                     [](const auto &tx) { return clone(tx); });
      return validate(res);
    }

    template <typename TransactionValidator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const shared_model::interface::types::SharedTxsCollectionType
            &transactions) const {
      Answer res;
      ReasonsGroupType reason;
      reason.first = "Transaction list";

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

    template <typename TransactionValidator>
    const TransactionValidator &TransactionsCollectionValidator<
        TransactionValidator>::getTransactionValidator() const {
      return transaction_validator_;
    }

    template class TransactionsCollectionValidator<DefaultTransactionValidator>;

    template class TransactionsCollectionValidator<
        DefaultSignableTransactionValidator>;

  }  // namespace validation
}  // namespace shared_model
