/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "utils/string_builder.hpp"
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
      if (txs.empty()) {
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

    template <typename TransactionValidator, typename OrderValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<TransactionValidator,
                                                          OrderValidator>
            &validator) {
      auto answer = validator.validatePointers(transactions);
      if (not allTxsInSameBatch(transactions)) {
        answer.addReason(std::make_pair(
            "Transaction batch: ",
            std::vector<std::string>{
                "Provided transactions are not from the same batch"}));
      }

      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionBatch(transactions));
    }

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<
            validation::TransactionValidator<
                validation::FieldValidator,
                validation::CommandValidatorVisitor<
                    validation::FieldValidator>>,
            validation::BatchOrderValidator> &validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::TransactionsCollectionValidator<
            validation::TransactionValidator<
                validation::FieldValidator,
                validation::CommandValidatorVisitor<
                    validation::FieldValidator>>,
            validation::AnyOrderValidator> &validator);

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
        const validation::TransactionValidator<
            validation::FieldValidator,
            validation::CommandValidatorVisitor<validation::FieldValidator>>
            &validator);

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

    std::string TransactionBatch::toString() const {
      return detail::PrettyStringBuilder()
          .init("Batch")
          .append("reducedHash", reducedHash().toString())
          .append("hasAllSignatures", hasAllSignatures() ? "true" : "false")
          .append("transactions")
          .appendAll(transactions(), [](auto &tx) { return tx->toString(); })
          .finalize();
    }

  }  // namespace interface
}  // namespace shared_model
