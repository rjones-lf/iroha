/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/transactions_collection/transactions_collection_validator.hpp"

#include <algorithm>

#include <boost/format.hpp>
#include <boost/range/adaptor/indirected.hpp>
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
    template <typename Validator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validateImpl(
        const interface::types::TransactionsForwardCollectionType &transactions,
        Validator &&validator) const {
      Answer res;
      ReasonsGroupType reason;
      reason.first = "Transaction list";

      if (boost::empty(transactions)) {
        reason.second.emplace_back("Transaction sequence can not be empty");
        res.addReason(std::move(reason));
        return res;
      }

      for (const auto &tx : transactions) {
        auto answer = std::forward<Validator>(validator)(tx);
        if (answer.hasErrors()) {
          auto message =
              (boost::format("Tx %s : %s") % tx.hash().hex() % answer.reason())
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
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const shared_model::interface::types::TransactionsForwardCollectionType
            &transactions) const {
      return validateImpl(transactions, [this](const auto &tx) {
        return transaction_validator_.validate(tx);
      });
    }

    template <typename TransactionValidator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const shared_model::interface::types::SharedTxsCollectionType
            &transactions) const {
      return validate(transactions | boost::adaptors::indirected);
    }

    template <typename TransactionValidator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const interface::types::TransactionsForwardCollectionType &transactions,
        interface::types::TimestampType current_timestamp) const {
      return validateImpl(
          transactions, [this, current_timestamp](const auto &tx) {
            return transaction_validator_.validate(tx, current_timestamp);
          });
    }

    template <typename TransactionValidator>
    Answer TransactionsCollectionValidator<TransactionValidator>::validate(
        const interface::types::SharedTxsCollectionType &transactions,
        interface::types::TimestampType current_timestamp) const {
      return validate(transactions | boost::adaptors::indirected,
                      current_timestamp);
    }

    template <typename TransactionValidator>
    const TransactionValidator &TransactionsCollectionValidator<
        TransactionValidator>::getTransactionValidator() const {
      return transaction_validator_;
    }

    template class TransactionsCollectionValidator<
        DefaultUnsignedTransactionValidator>;

    template class TransactionsCollectionValidator<
        DefaultSignedTransactionValidator>;

  }  // namespace validation
}  // namespace shared_model
