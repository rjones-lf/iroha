/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include "framework/batch_helper.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"
#include "validators/transactions_collection/unsigned_transactions_collection_validator.hpp"

using namespace shared_model;
using ::testing::_;
using ::testing::Return;
using ::testing::Test;
using ::testing::Truly;

using TxValidator = validation::TransactionValidator<
    validation::FieldValidator,
    validation::CommandValidatorVisitor<validation::FieldValidator>>;

using TxsValidator = validation::UnsignedTransactionsCollectionValidator<
    TxValidator,
    validation::BatchOrderValidator>;

/**
 * Creates valid unsigned transaction
 * @param created_time assigned to transactions
 * @return std::shared_ptr<interface::Transaction> containing valid unsigned
 * transaction
 */
auto createValidUnsignedTransaction(size_t created_time = iroha::time::now()) {
  return std::shared_ptr<interface::Transaction>(clone(
      framework::batch::prepareTransactionBuilder("valid@account", created_time)
          .build()));
}

/**
 * Creates invalid unsigned transaction
 * @param created_time assigned to transactions
 * @return std::shared_ptr<interface::Transaction> containing invalid unsigned
 * transaction
 */
auto createInvalidUnsignedTransaction(
    size_t created_time = iroha::time::now()) {
  return std::shared_ptr<interface::Transaction>(
      clone(framework::batch::prepareTransactionBuilder("invalid#@account",
                                                        created_time)
                .build()));
}

/**
 * @given valid transactions sequence from the single batch
 * @when createTransactionBatch is invoked on that sequence
 * @then transaction batch is created
 */
TEST(TransactionBatchTest, CreateTransactionBatchWhenValid) {
  auto txs = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC,
      std::vector<std::string>{"a@domain", "b@domain"});
  auto transaction_batch =
      interface::TransactionBatch::createTransactionBatch(txs, TxsValidator());
  ASSERT_TRUE(framework::expected::val(transaction_batch))
      << framework::expected::err(transaction_batch).value().error;
}

/**
 * @given transactions sequence from the single batch containing valid
 * transactions but with different batch types
 * @when createTransactionBatch is invoked on that sequence
 * @then transaction batch is not created
 */
TEST(TransactionBatchTest, CreateTransactionBatchWhenDifferentBatchType) {
  auto tx1_fields = std::make_pair(interface::types::BatchType::ORDERED,
                                   std::string("a@domain"));
  auto tx2_fields = std::make_pair(interface::types::BatchType::ATOMIC,
                                   std::string("b@domain"));

  auto txs = framework::batch::createUnsignedBatchTransactions(
      std::vector<decltype(tx1_fields)>{tx1_fields, tx2_fields});

  auto transaction_batch =
      interface::TransactionBatch::createTransactionBatch(txs, TxsValidator());
  ASSERT_TRUE(framework::expected::err(transaction_batch));
}

/**
 * @given transactions sequence from the single batch containing one valid and
 * one invalid transactions
 * @when createTransactionBatch is invoked on that sequence
 * @then transaction batch is not created
 */
TEST(TransactionBatchTest, CreateBatchWithValidAndInvalidTx) {
  auto txs = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC,
      std::vector<std::string>{"valid@name", "invalid#@name"});

  auto transaction_batch =
      interface::TransactionBatch::createTransactionBatch(txs, TxsValidator());
  ASSERT_TRUE(framework::expected::err(transaction_batch));
}

/**
 * @given single valid transaction
 * @when createTransactionBatch is invoked on that transaction
 * @then transaction batch is created
 */
TEST(TransactionBatchTest, CreateSingleTxBatchWhenValid) {
  TxValidator transaction_validator;

  auto tx1 = createValidUnsignedTransaction();

  auto transaction_batch = interface::TransactionBatch::createTransactionBatch(
      tx1, transaction_validator);

  ASSERT_TRUE(framework::expected::val(transaction_batch))
      << framework::expected::err(transaction_batch).value().error;
}

/**
 * @given single invalid transaction
 * @when createTransactionBatch is invoked on that transaction
 * @then transaction batch is not created
 */
TEST(TransactionBatchTest, CreateSingleTxBatchWhenInvalid) {
  TxValidator transaction_validator;

  auto tx1 = createInvalidUnsignedTransaction();

  auto transaction_batch = interface::TransactionBatch::createTransactionBatch(
      tx1, transaction_validator);

  ASSERT_TRUE(framework::expected::err(transaction_batch));
}
