/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include "framework/batch_helper.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"

using namespace shared_model;
using ::testing::_;
using ::testing::Return;
using ::testing::Test;

class MockTransactionCollectionValidator
    : public validation::UnsignedTransactionsCollectionValidator<
          validation::TransactionValidator<validation::FieldValidator,
                                           validation::CommandValidatorVisitor<
                                               validation::FieldValidator>>> {
 public:
  MOCK_CONST_METHOD1(
      validatePointers,
      validation::Answer(const interface::types::SharedTxsCollectionType &));
};

shared_model::validation::Answer createAnswerWithErrors() {
  shared_model::validation::Answer answer;
  answer.addReason(
      std::make_pair("transaction", std::vector<std::string>{"some reason"}));
  return answer;
}

/**
 * @given Transaction collection of several transactions
 * @when create transaction sequence
 * @and transactions validator returns empty answer
 * @then TransactionSequence is created
 */
TEST(TransactionSequenceTest, CreateTransactionSequenceWhenValid) {
  MockTransactionCollectionValidator transactions_validator;

  EXPECT_CALL(transactions_validator, validatePointers(_))
      .WillOnce(Return(validation::Answer()));

  std::shared_ptr<interface::Transaction> tx(clone(
      framework::batch::prepareTransactionBuilder("account@domain")
          .batchMeta(shared_model::interface::types::BatchType::ATOMIC,
                     std::vector<shared_model::interface::types::HashType>{})
          .build()));

  auto tx_sequence = interface::TransactionSequence::createTransactionSequence(
      std::vector<decltype(tx)>{tx, tx, tx}, transactions_validator);

  ASSERT_TRUE(framework::expected::val(tx_sequence));
}

/**
 * @given Transaction collection of several transactions
 * @when create transaction sequence
 * @and transactions validator returns non empty answer
 * @then TransactionSequence is not created
 */
TEST(TransactionSequenceTest, CreateTransactionSequenceWhenInvalid) {
  MockTransactionCollectionValidator res;

  EXPECT_CALL(res, validatePointers(_))
      .WillOnce(Return(createAnswerWithErrors()));

  std::shared_ptr<interface::Transaction> tx(clone(
      framework::batch::prepareTransactionBuilder("account@domain")
          .batchMeta(shared_model::interface::types::BatchType::ATOMIC,
                     std::vector<shared_model::interface::types::HashType>{})
          .build()));

  auto tx_sequence = interface::TransactionSequence::createTransactionSequence(
      std::vector<decltype(tx)>{tx, tx, tx}, res);

  ASSERT_TRUE(framework::expected::err(tx_sequence));
}

/**
 * @given Transaction collection of several transactions, including some of the
 * united into the batches
 * @when transactions validator returns empty answer
 * @and create transaction sequence
 * @then expected number of batches is created
 */
TEST(TransactionSequenceTest, CreateBatches) {
  size_t batches_number = 3;
  size_t txs_in_batch = 2;
  size_t single_transactions = 1;

  MockTransactionCollectionValidator txs_validator;

  EXPECT_CALL(txs_validator, validatePointers(_))
      .Times(batches_number)
      .WillRepeatedly(Return(validation::Answer()));

  interface::types::SharedTxsCollectionType tx_collection;
  auto now = iroha::time::now();
  for (size_t i = 0; i < batches_number; i++) {
    auto batch = framework::batch::createUnsignedBatchTransactions(
        shared_model::interface::types::BatchType::ATOMIC,
        txs_in_batch,
        now + i);
    tx_collection.insert(tx_collection.begin(), batch.begin(), batch.end());
  }

  for (size_t i = 0; i < single_transactions; i++) {
    tx_collection.emplace_back(
        clone(framework::batch::prepareTransactionBuilder(
                  "single_tx_account@domain" + std::to_string(i))
                  .build()));
  }

  auto tx_sequence_opt =
      interface::TransactionSequence::createTransactionSequence(tx_collection,
                                                                txs_validator);

  auto tx_sequence = framework::expected::val(tx_sequence_opt);
  ASSERT_TRUE(tx_sequence)
      << framework::expected::err(tx_sequence_opt).value().error;

  ASSERT_EQ(boost::size(tx_sequence->value.batches()),
            batches_number + single_transactions);

  size_t total_transactions = boost::accumulate(
      tx_sequence->value.batches(), 0ul, [](auto sum, const auto &batch) {
        return sum + boost::size(batch.transactions());
      });
  ASSERT_EQ(total_transactions,
            batches_number * txs_in_batch + single_transactions);
}
