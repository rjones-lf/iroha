/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"

#include <boost/range/combine.hpp>

#include "interfaces/iroha_internal/batch_meta.hpp"
#include "interfaces/iroha_internal/transaction_batch_impl.hpp"
#include "interfaces/transaction.hpp"
#include "validators/answer.hpp"

namespace {
  enum class BatchCheckResult {
    kOk,
    kNoBatchMeta,
    kIncorrectBatchMetaSize,
    kIncorrectHashes
  };
  /**
   * Check that all transactions from the collection are mentioned in batch_meta
   * and are positioned correctly
   * @param transactions to be checked
   * @return enum, reporting about success result or containing a found error
   */
  BatchCheckResult batchIsWellFormed(
      const shared_model::interface::types::SharedTxsCollectionType
          &transactions) {
    auto batch_meta_opt = transactions[0]->batchMeta();
    if (not batch_meta_opt and transactions.size() == 1) {
      // batch is created from one tx - there is no batch_meta in valid case
      return BatchCheckResult::kOk;
    }
    if (not batch_meta_opt) {
      // in all other cases batch_meta must present
      return BatchCheckResult::kNoBatchMeta;
    }

    const auto &batch_hashes = batch_meta_opt->get()->reducedHashes();
    if (batch_hashes.size() != transactions.size()) {
      return BatchCheckResult::kIncorrectBatchMetaSize;
    }

    auto metas_and_txs = boost::combine(batch_hashes, transactions);
    auto hashes_are_correct =
        std::all_of(boost::begin(metas_and_txs),
                    boost::end(metas_and_txs),
                    [](const auto &meta_and_tx) {
                      shared_model::interface::types::HashType batch_hash;
                      std::shared_ptr<shared_model::interface::Transaction> tx;
                      boost::tie(batch_hash, tx) = meta_and_tx;
                      return batch_hash == tx->reducedHash();
                    });
    if (not hashes_are_correct) {
      return BatchCheckResult::kIncorrectHashes;
    }

    return BatchCheckResult::kOk;
  }
}  // namespace

namespace shared_model {
  namespace interface {
    TransactionBatchFactoryImpl::FactoryImplResult
    TransactionBatchFactoryImpl::createTransactionBatch(
        const types::SharedTxsCollectionType &transactions) const {
      std::string reason_name = "Transaction batch factory: ";
      validation::ReasonsGroupType batch_reason;
      batch_reason.first = reason_name;

      bool has_at_least_one_signature = std::any_of(
          transactions.begin(), transactions.end(), [](const auto tx) {
            return not boost::empty(tx->signatures());
          });
      if (not has_at_least_one_signature) {
        batch_reason.second.emplace_back(
            "Transaction batch should contain at least one signature");
      }

      switch (batchIsWellFormed(transactions)) {
        case BatchCheckResult::kOk:
          break;
        case BatchCheckResult::kNoBatchMeta:
          batch_reason.second.emplace_back(
              "There is no batch meta in provided transactions");
          break;
        case BatchCheckResult::kIncorrectBatchMetaSize:
          batch_reason.second.emplace_back(
              "Sizes of batch_meta and provided transactions are different");
          break;
        case BatchCheckResult::kIncorrectHashes:
          batch_reason.second.emplace_back(
              "Hashes of provided transactions and ones in batch_meta are "
              "different");
          break;
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

    TransactionBatchFactoryImpl::FactoryImplResult
    TransactionBatchFactoryImpl::createTransactionBatch(
        std::shared_ptr<Transaction> transaction) const {
      return createTransactionBatch(
          types::SharedTxsCollectionType{std::move(transaction)});
    }
  }  // namespace interface
}  // namespace shared_model
