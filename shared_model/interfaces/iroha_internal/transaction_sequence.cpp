/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace interface {

    template <typename TransactionValidator>
    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator>
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
                    validation::FieldValidator>>> &validator);

    types::SharedTxsCollectionType TransactionSequence::transactions() {
      return transactions_;
    }

    const types::BatchesType &TransactionSequence::batches() {
      return *batches_;
    }

    TransactionSequence::TransactionSequence(
        const types::SharedTxsCollectionType &transactions)
        : transactions_(transactions), batches_{[this]() -> types::BatchesType {
            std::unordered_map<std::string,
                               std::vector<std::shared_ptr<Transaction>>>
                extracted_batches;
            std::vector<types::SharedTxsCollectionType> batches;
            for (const auto &tx : transactions_) {
              if (auto meta = tx->batch_meta()) {
                auto hashes = meta.get()->transactionHashes();
                auto batch_hash = this->calculateBatchHash(hashes);
                extracted_batches[batch_hash].push_back(tx);
              } else {
                batches.push_back(std::vector<std::shared_ptr<Transaction>>{tx});
              }
            }
            for (auto it : extracted_batches) {
              batches.push_back(it.second);
            }
            types::SharedTxsCollectionType s = batches.at(0);
            return batches;
          }} {}

    std::string TransactionSequence::calculateBatchHash(
        std::vector<types::HashType> reduced_hashes) {
      std::stringstream concatenated_hashes_stream;
      for (const auto &hash : reduced_hashes) {
        concatenated_hashes_stream << hash.hex();
      }
      return concatenated_hashes_stream.str();
    }

  }  // namespace interface
}  // namespace shared_model
