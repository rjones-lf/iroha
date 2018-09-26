/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "interfaces/iroha_internal/transaction_batch_unsafe_factory.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model::interface;

class TransactionBatchUnsafeFactoryTest : public ::testing::Test {
 public:
  std::shared_ptr<TransactionBatchUnsafeFactory> factory_ =
      std::make_shared<TransactionBatchUnsafeFactory>();

  /**
   * Create a transaction with some data
   * @param metadata to make hash unique
   * @return shared_ptr to that transaction
   */
  std::unique_ptr<Transaction> createTransaction(int metadata) {
    return std::make_unique<shared_model::proto::Transaction>(
        TestTransactionBuilder()
            .setAccountQuorum("doge@meme", metadata)
            .build());
  }

  /**
   * Create multiple transactions without any data
   * @param tx_number - desired number of transactions
   * @return vector of shared_ptr to those transactions
   */
  types::SharedTxsCollectionType createTransactions(int tx_number) {
    types::SharedTxsCollectionType txs;
    for (auto i = 0; i < tx_number; ++i) {
      txs.push_back(createTransaction(i + 1));
    }
    return txs;
  }
};

/**
 * @given single transaction
 * @when creating a batch via factory from it
 * @then the batch is well-formed @and contains this transaction
 */
TEST_F(TransactionBatchUnsafeFactoryTest, CreateBatchFromSingleTx) {
  auto batch_tx = createTransaction(1);
  auto batch_tx_hash = batch_tx->reducedHash();

  auto batch = factory_->createTransactionBatch(std::move(batch_tx));

  ASSERT_EQ(batch->transactions()[0]->reducedHash(), batch_tx_hash);
}

/**
 * @given a collection of transactions
 * @when creating a batch via factory from it
 * @then the batch is well-formed @and contains those transactions
 */
TEST_F(TransactionBatchUnsafeFactoryTest, CreateBatchFromMultipleTxs) {
  constexpr int kBatchSize = 10;
  auto batches = createTransactions(kBatchSize);

  auto batch = factory_->createTransactionBatch(batches);

  for (auto i = 0; i < kBatchSize; i++) {
    ASSERT_EQ(batch->transactions()[i]->reducedHash(),
              batches[i]->reducedHash());
  }
}
