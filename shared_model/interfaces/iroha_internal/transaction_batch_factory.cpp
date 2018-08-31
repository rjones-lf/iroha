/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_factory.hpp"

#include "validators/default_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

namespace shared_model {
  namespace interface {

    /**
     * check if all transactions belong to the same batch
     * @param txs transactions to be checked
     * @return true if all transactions from the same batch and false otherwise
     */
    static bool allTxsInSameBatch(const types::SharedTxsCollectionType &txs) {
      if (txs.size() == 1) {
        return true;
      }

      // take batch meta of the first transaction and compare it with batch
      // metas of remaining transactions
      auto batch_meta = txs.front()->batchMeta();
      if (not batch_meta) {
        return false;
      }

      return std::none_of(++txs.begin(),
                          txs.end(),
                          [front_batch_meta = batch_meta.value()](
                              const std::shared_ptr<Transaction> tx) {
                            return tx->batchMeta()
                                ? **tx->batchMeta() != *front_batch_meta
                                : false;
                          });
    }

    template <typename TransactionValidator, typename FieldValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator>
            &validator,
        const FieldValidator &field_validator) {
      auto answer = validator.validate(transactions);

      std::string reason_name = "Transaction batch: ";
      validation::ReasonsGroupType batch_reason;
      batch_reason.first = reason_name;
      if (boost::empty(transactions)) {
        batch_reason.second.emplace_back(
            "Provided transactions are not from the same batch");
      }
      if (not allTxsInSameBatch(transactions)) {
        batch_reason.second.emplace_back(
            "Provided transactions are not from the same batch");
      }
      bool has_at_least_one_signature =
          std::any_of(transactions.begin(),
                      transactions.end(),
                      [&field_validator, &batch_reason](const auto tx) {
                        const auto &signatures = tx->signatures();
                        if (not boost::empty(signatures)) {
                          field_validator.validateSignatures(
                              batch_reason, signatures, tx->payload());
                          return true;
                        }
                        return false;
                      });

      if (not has_at_least_one_signature) {
        batch_reason.second.emplace_back(
            "Transaction batch should contain at least one signature");
      }

      if (not batch_reason.second.empty()) {
        answer.addReason(std::move(batch_reason));
      }

      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionBatch(transactions));
    }

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultUnsignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultSignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template <typename TransactionValidator, typename FieldValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const TransactionValidator &transaction_validator,
        const FieldValidator &field_validator) {
      validation::ReasonsGroupType reason;
      reason.first = "Transaction batch: ";
      field_validator.validateSignatures(
          reason, transaction->signatures(), transaction->payload());

      auto answer = transaction_validator.validate(*transaction);
      if (answer.hasErrors()) {
        answer.addReason(std::move(reason));
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(
          TransactionBatch(types::SharedTxsCollectionType{transaction}));
    };

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultUnsignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatchFactory::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultSignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

  }  // namespace interface
}  // namespace shared_model
