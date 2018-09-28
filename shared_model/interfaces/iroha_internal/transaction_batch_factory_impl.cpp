/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"

#include "interfaces/iroha_internal/transaction_batch_impl.hpp"
#include "interfaces/transaction.hpp"
#include "validators/answer.hpp"

shared_model::interface::TransactionBatchFactoryImpl::FactoryImplResult
shared_model::interface::TransactionBatchFactoryImpl::createTransactionBatch(
    const types::SharedTxsCollectionType &transactions) const {
  std::string reason_name = "Transaction batch: ";
  validation::ReasonsGroupType batch_reason;
  batch_reason.first = reason_name;

  if (boost::empty(transactions)) {
    batch_reason.second.emplace_back("Provided transactions list is empty");
  }

  bool has_at_least_one_signature =
      std::any_of(transactions.begin(), transactions.end(), [](const auto tx) {
        return not boost::empty(tx->signatures());
      });
  if (not has_at_least_one_signature) {
    batch_reason.second.emplace_back(
        "Transaction batch should contain at least one signature");
  }

  if (not batch_reason.second.empty()) {
    validation::Answer answer;
    answer.addReason(std::move(batch_reason));
    return iroha::expected::makeError(answer.reason());
  }

  std::unique_ptr<TransactionBatch> batch_ptr =
      std::make_unique<TransactionBatchImpl>(transactions);
  return iroha::expected::makeValue(std::move(batch_ptr));
}

shared_model::interface::TransactionBatchFactoryImpl::FactoryImplResult
shared_model::interface::TransactionBatchFactoryImpl::createTransactionBatch(
    std::shared_ptr<Transaction> transaction) const {
  validation::ReasonsGroupType reason;
  reason.first = "Transaction batch: ";

  if (boost::empty(transaction->signatures())) {
    reason.second.emplace_back(
        "Transaction should contain at least one signature");
  }

  if (not reason.second.empty()) {
    validation::Answer answer;
    answer.addReason(std::move(reason));
    return iroha::expected::makeError(answer.reason());
  }

  std::unique_ptr<TransactionBatch> batch_ptr =
      std::make_unique<TransactionBatchImpl>(
          types::SharedTxsCollectionType{transaction});
  return iroha::expected::makeValue(std::move(batch_ptr));
}
