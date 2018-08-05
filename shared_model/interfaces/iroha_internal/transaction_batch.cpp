/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch.hpp"
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
      // Empty batch is still batch, so txs can be empty
      if (txs.empty() or txs.size() == 1) {
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
    };

    template <typename TransactionValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator>
            &validator) {
      auto answer = validator.validate(transactions);
      std::string reason_name = "Transaction batch: ";
      validation::ReasonsGroupType batch_reason;
      batch_reason.first = reason_name;
      if (not allTxsInSameBatch(transactions)) {
        batch_reason.second.emplace_back(
            "Provided transactions are not from the same batch");
      }
      bool has_at_least_one_signature = std::any_of(
          transactions.begin(), transactions.end(), [](const auto tx) {
            return not boost::empty(tx->signatures());
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
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultUnsignedTransactionsValidator &validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultSignedTransactionsValidator &validator);

    template <typename TransactionValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const TransactionValidator &transaction_validator) {
      auto answer = transaction_validator.validate(*transaction);
      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(
          TransactionBatch(types::SharedTxsCollectionType{transaction}));
    };

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultTransactionValidator &validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultSignableTransactionValidator &validator);

    const types::SharedTxsCollectionType &TransactionBatch::transactions()
        const {
      return transactions_;
    }

    const types::HashType &TransactionBatch::reducedHash() const {
      if (not reduced_hash_) {
        reduced_hash_ = TransactionBatch::calculateReducedBatchHash(
            transactions_ | boost::adaptors::transformed([](const auto &tx) {
              return tx->reducedHash();
            }));
      }
      return reduced_hash_.value();
    }

    bool TransactionBatch::hasAllSignatures() const {
      return std::all_of(
          transactions_.begin(), transactions_.end(), [](const auto tx) {
            return boost::size(tx->signatures()) >= tx->quorum();
          });
    }

    types::HashType TransactionBatch::calculateReducedBatchHash(
        const boost::any_range<types::HashType, boost::forward_traversal_tag>
            &reduced_hashes) {
      std::stringstream concatenated_hash;
      for (const auto &hash : reduced_hashes) {
        concatenated_hash << hash.hex();
      }
      return types::HashType::fromHexString(concatenated_hash.str());
    }

  }  // namespace interface
}  // namespace shared_model
