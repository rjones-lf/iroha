/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch.hpp"

#include <algorithm>

#include "utils/string_builder.hpp"
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
    };

    template <typename TransactionValidator, typename FieldValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
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
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultUnsignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions,
        const validation::DefaultSignedTransactionsValidator &validator,
        const validation::FieldValidator &field_validator);

    // TODO: 11/08/2018 @muratovv move to own hpp file IR-1595
    template <typename TransactionValidator, typename FieldValidator>
    iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
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

    // TODO: 11/08/2018 @muratovv move instantiation to batch_helper.hpp IR-1595
    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultUnsignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

    template iroha::expected::Result<TransactionBatch, std::string>
    TransactionBatch::createTransactionBatch(
        std::shared_ptr<Transaction> transaction,
        const validation::DefaultSignedTransactionValidator
            &transaction_validator,
        const validation::FieldValidator &field_validator);

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

    bool TransactionBatch::addSignature(
        size_t number_of_tx,
        const shared_model::crypto::Signed &signed_blob,
        const shared_model::crypto::PublicKey &public_key) {
      if (number_of_tx >= transactions_.size()) {
        return false;
      } else {
        return transactions_.at(number_of_tx)
            ->addSignature(signed_blob, public_key);
      }
    }

    bool TransactionBatch::operator==(const TransactionBatch &rhs) const {
      return reducedHash() == rhs.reducedHash()
          and std::equal(transactions().begin(),
                         transactions().end(),
                         rhs.transactions().begin(),
                         rhs.transactions().end(),
                         [](auto const &left, auto const &right) {
                           return left->equalsByValue(*right);
                         });
    }

  }  // namespace interface
}  // namespace shared_model
