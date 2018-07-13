/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/any_order_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

namespace shared_model {
  namespace interface {

    template <typename TransactionValidator, typename OrderValidator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator,
                                                          OrderValidator>
            &validator) {
      auto answer = validator.validatePointers(transactions);
      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionSequence(transactions));
    }

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<
            validation::TransactionValidator<
                validation::FieldValidator,
                validation::CommandValidatorVisitor<
                    validation::FieldValidator>>,
            validation::AnyOrderValidator> &validator);

    template iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<
            validation::TransactionValidator<
                validation::FieldValidator,
                validation::CommandValidatorVisitor<
                    validation::FieldValidator>>,
            validation::BatchOrderValidator> &validator);

    types::SharedTxsCollectionType TransactionSequence::transactions() const {
      return transactions_;
    }

    TransactionSequence::TransactionSequence(
        const types::SharedTxsCollectionType &transactions)
        : transactions_(transactions) {}

    types::BatchesType TransactionSequence::batches() const {
      if (batches_) {
        return batches_.value();
      }

      std::unordered_map<std::string, std::vector<std::shared_ptr<Transaction>>>
          extracted_batches;
      std::vector<types::SharedTxsCollectionType> batches;
      for (const auto &tx : transactions_) {
        if (auto meta = tx->batchMeta()) {
          auto hashes = meta.get()->transactionHashes();
          auto batch_hash = this->calculateBatchHash(hashes);
          extracted_batches[batch_hash].push_back(tx);
        } else {
          batches.emplace_back(std::vector<std::shared_ptr<Transaction>>{tx});
        }
      }
      for (auto it : extracted_batches) {
        batches.emplace_back(it.second);
      }

      batches_.emplace(batches);
      return batches;
    }

    std::string TransactionSequence::calculateBatchHash(
        std::vector<types::HashType> reduced_hashes) const {
      std::stringstream concatenated_hashes_stream;
      for (const auto &hash : reduced_hashes) {
        concatenated_hashes_stream << hash.hex();
      }
      return concatenated_hashes_stream.str();
    }

  }  // namespace interface
}  // namespace shared_model
