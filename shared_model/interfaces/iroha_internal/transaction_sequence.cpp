/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"

#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace interface {

    template <typename TransactionValidator, typename FieldValidator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator>
            &validator,
        const FieldValidator &field_validator) {
      std::unordered_map<interface::types::HashType,
                         std::vector<std::shared_ptr<Transaction>>,
                         interface::types::HashType::Hasher>
          extracted_batches;
      std::vector<TransactionBatch> batches;

      const auto &transaction_validator = validator.getTransactionValidator();

      auto insert_batch =
          [&batches](const iroha::expected::Value<TransactionBatch> &value) {
            batches.push_back(value.value);
          };

      validation::Answer result;
      if (transactions.size() == 0) {
        result.addReason(std::make_pair(
            "Transaction collection error",
            std::vector<std::string>{"sequence can not be empty"}));
      }
      for (const auto &tx : transactions) {
        if (auto meta = tx->batchMeta()) {
          auto hashes = meta.get()->reducedHashes();
          auto batch_hash = TransactionBatch::calculateReducedBatchHash(hashes);
          extracted_batches[batch_hash].push_back(tx);
        } else {
          TransactionBatch::createTransactionBatch<TransactionValidator,
                                                   FieldValidator>(
              tx, transaction_validator, field_validator)
              .match(insert_batch, [&tx, &result](const auto &err) {
                result.addReason(std::make_pair(
                    std::string("Error in transaction with reduced hash: ")
                        + tx->reducedHash().hex(),
                    std::vector<std::string>{err.error}));
              });
        }
      }

      for (const auto &it : extracted_batches) {
        TransactionBatch::createTransactionBatch(it.second, validator)
            .match(insert_batch, [&it, &result](const auto &err) {
              result.addReason(std::make_pair(
                  it.first.toString(), std::vector<std::string>{err.error}));
            });
      }

      if (result.hasErrors()) {
        return iroha::expected::makeError(result.reason());
      }

      return iroha::expected::makeValue(TransactionSequence(batches));
    }

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultUnsignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultSignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    const types::SharedTxsCollectionType &TransactionSequence::transactions()
        const {
      if (not transactions_) {
        types::SharedTxsCollectionType result;
        auto transactions_amount = 0u;
        for (const auto &batch : batches_) {
          transactions_amount += batch.transactions().size();
        }
        result.reserve(transactions_amount);
        for (const auto &batch : batches_) {
          auto &transactions = batch.transactions();
          std::copy(transactions.begin(),
                    transactions.end(),
                    std::back_inserter(result));
        }
        transactions_.emplace(std::move(result));
      }
      return transactions_.value();
    }

    const types::BatchesCollectionType &TransactionSequence::batches() const {
      return batches_;
    }

    std::string TransactionSequence::toString() const {
      return detail::PrettyStringBuilder()
          .init("TransactionSequence")
          .appendAll(batches_,
                     [](const auto &batch) { return batch.toString(); })
          .finalize();
    }

    TransactionSequence::TransactionSequence(
        const types::BatchesCollectionType &batches)
        : batches_(batches) {}

  }  // namespace interface
}  // namespace shared_model
