/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BATCH_HELPER_HPP
#define IROHA_BATCH_HELPER_HPP

#include <boost/range/irange.hpp>

#include "framework/result_fixture.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

namespace framework {
  namespace batch {

    /**
     * Creates transaction builder with set creator
     * @tparam TransactionBuilderType type of tranasction builder
     * @return prepared transaction builder
     */
    template <typename TransactionBuilderType = TestTransactionBuilder>
    auto prepareTransactionBuilder(
        const std::string &creator,
        const size_t &created_time = iroha::time::now(),
        const shared_model::interface::types::QuorumType &quorum = 1) {
      return TransactionBuilderType()
          .setAccountQuorum(creator, 1)
          .creatorAccountId(creator)
          .createdTime(created_time)
          .quorum(quorum);
    }

    /**
     * Creates unsigned transaction builder with set creator
     * @return prepared transaction builder
     */
    auto prepareUnsignedTransactionBuilder(
        const std::string &creator,
        const size_t &created_time = iroha::time::now(),
        const shared_model::interface::types::QuorumType &quorum = 1) {
      return prepareTransactionBuilder<TestUnsignedTransactionBuilder>(
          creator, created_time, quorum);
    }

    /**
     * Create unsigned batch with given fields of transactions: batch type and
     * creator account.
     * @param btype_creator_pairs vector of pairs. First element in every pair
     * is batch type and second is creator account
     * @param now created time for every transaction
     * @return batch with the same size as size of range of pairs
     */
    auto createUnsignedBatchTransactions(
        std::vector<std::pair<shared_model::interface::types::BatchType,
                              std::string>> btype_creator_pairs,
        size_t now = iroha::time::now()) {
      std::vector<shared_model::interface::types::HashType> reduced_hashes;
      for (const auto &btype_creator : btype_creator_pairs) {
        auto tx = prepareTransactionBuilder(btype_creator.second, now).build();
        reduced_hashes.push_back(tx.reducedHash());
      }

      shared_model::interface::types::SharedTxsCollectionType txs;

      std::transform(
          btype_creator_pairs.begin(),
          btype_creator_pairs.end(),
          std::back_inserter(txs),
          [&now, &reduced_hashes](const auto &btype_creator)
              -> shared_model::interface::types::SharedTxsCollectionType::
                  value_type {
                    return clone(
                        prepareTransactionBuilder(btype_creator.second, now)
                            .batchMeta(btype_creator.first, reduced_hashes)
                            .build());
                  });
      return txs;
    }

    /**
     * Creates batch transactions, where every transaction has single signature
     * @param btype_creator_pairs vector of pairs of batch type and creator
     * account id
     * @param now created time for every transaction
     * @param quorum quorum for every transaction
     * @return batch with the same size as size of range of pairs
     */
    auto createBatchOneSignTransactions(
        std::vector<std::pair<shared_model::interface::types::BatchType,
                              std::string>> btype_creator_pairs,
        size_t now = iroha::time::now(),
        const shared_model::interface::types::QuorumType &quorum = 1) {
      std::vector<shared_model::interface::types::HashType> reduced_hashes;
      for (const auto &btype_creator : btype_creator_pairs) {
        auto tx =
            prepareUnsignedTransactionBuilder(btype_creator.second, now, quorum)
                .build();
        reduced_hashes.push_back(tx.reducedHash());
      }

      shared_model::interface::types::SharedTxsCollectionType txs;

      std::transform(
          btype_creator_pairs.begin(),
          btype_creator_pairs.end(),
          std::back_inserter(txs),
          [&now, &reduced_hashes, &quorum](const auto &btype_creator)
              -> shared_model::interface::types::SharedTxsCollectionType::
                  value_type {
                    return clone(
                        prepareUnsignedTransactionBuilder(
                            btype_creator.second, now, quorum)
                            .batchMeta(btype_creator.first, reduced_hashes)
                            .build()
                            .signAndAddSignature(
                                shared_model::crypto::
                                    DefaultCryptoAlgorithmType::
                                        generateKeypair())
                            .finish());
                  });
      return txs;
    }

    /**
     * Creates atomic batch from provided creator accounts
     * @param creators vector of creator account ids
     * @return unsigned batch of the same size as the size of creator account
     * ids
     */
    auto createUnsignedBatchTransactions(
        shared_model::interface::types::BatchType batch_type,
        const std::vector<std::string> &creators,
        size_t now = iroha::time::now()) {
      std::vector<std::pair<decltype(batch_type), std::string>> fields;
      std::transform(creators.begin(),
                     creators.end(),
                     std::back_inserter(fields),
                     [&batch_type](const auto creator) {
                       return std::make_pair(batch_type, creator);
                     });
      return createUnsignedBatchTransactions(fields, now);
    }

    /**
     * Creates transaction collection for the batch of given type and size
     * @param batch_type type of the creted transactions
     * @param batch_size size of the created collection of transactions
     * @param now created time for every transactions
     * @return unsigned batch
     */
    auto createUnsignedBatchTransactions(
        shared_model::interface::types::BatchType batch_type,
        uint32_t batch_size,
        size_t now = iroha::time::now()) {
      auto range = boost::irange(0, (int)batch_size);
      std::vector<std::string> creators;

      std::transform(range.begin(),
                     range.end(),
                     std::back_inserter(creators),
                     [](const auto &id) {
                       return std::string("account") + std::to_string(id)
                           + "@domain";
                     });

      return createUnsignedBatchTransactions(batch_type, creators, now);
    }

    auto createValidBatch(const size_t &size) {
      using namespace shared_model::validation;
      using TxValidator =
          TransactionValidator<FieldValidator,
                               CommandValidatorVisitor<FieldValidator>>;

      using TxsValidator =
          UnsignedTransactionsCollectionValidator<TxValidator,
                                                  BatchOrderValidator>;

      auto batch_type = shared_model::interface::types::BatchType::ATOMIC;
      std::vector<std::pair<decltype(batch_type), std::string>>
          transaction_fields;
      for (size_t i = 0; i < size; ++i) {
        transaction_fields.push_back(std::make_pair(
            batch_type, "account" + std::to_string(i) + "@domain"));
      }

      auto txs = createBatchOneSignTransactions(transaction_fields);
      auto result_batch =
          shared_model::interface::TransactionBatch::createTransactionBatch(
              txs, TxsValidator());

      return framework::expected::val(result_batch).value().value;
    }

  }  // namespace batch
}  // namespace framework

#endif  // IROHA_BATCH_HELPER_HPP
