/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/batch_helper.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "utils/query_error_response_visitor.hpp"

constexpr auto kUser = "user@test";
constexpr auto kAsset = "asset#domain";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

/**
 * @given GetAccount query with non-existing user
 * AND default-initialized IntegrationTestFramework
 * @when query is sent to the framework
 * @then query response is ErrorResponse with STATEFUL_INVALID reason
 */
TEST(PipelineIntegrationTest, SendQuery) {
  auto query = shared_model::proto::QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId(kUser)
                   .queryCounter(1)
                   .getAccount(kUser)
                   .build()
                   .signAndAddSignature(
                       // TODO: 30/03/17 @l4l use keygen adapter IR-1189
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair())
                   .finish();

  auto check = [](auto &status) {
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::QueryErrorResponseChecker<
            shared_model::interface::StatefulFailedErrorResponse>(),
        status.get()));
  };
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendQuery(query, check)
      .done();
}

/**
 * prepares signed transaction with add asset quantity command
 * @param created_time created time of transaction, by default is now
 * @return Transaction with add asset quantity command
 */
auto prepareAddAssetQtyTransaction(size_t created_time = iroha::time::now()) {
  return shared_model::proto::TransactionBuilder()
      .createdTime(created_time)
      .creatorAccountId(kUser)
      .addAssetQuantity(kAsset, "1.0")
      .quorum(1)
      .build()
      .signAndAddSignature(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair())
      .finish();
}

/**
 * @given some user
 * @when sending sample AddAssetQuantity transaction to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status on that tx,
 * the tx is passed to proposal and does not appear in block
 */
TEST(PipelineIntegrationTest, SendTx) {
  auto tx = prepareAddAssetQtyTransaction();

  auto checkStatelessValid = [](auto &status) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::StatelessValidTxResponse>(),
        status.get()));
  };
  auto checkProposal = [](auto &proposal) {
    ASSERT_EQ(proposal->transactions().size(), 1);
  };
  auto checkBlock = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .checkProposal(checkProposal)
      .checkBlock(checkBlock)
      .done();
}

/**
 * prepares transaction sequence
 * @param tx_size the size of transaction sequence
 * @return  transaction sequence
 */
auto prepareTransactionSequence(size_t tx_size) {
  shared_model::interface::types::SharedTxsCollectionType txs;

  const auto &now = iroha::time::now();
  for (size_t i = 0; i < tx_size; i++) {
    auto &&tx = prepareAddAssetQtyTransaction(now + i);
    txs.push_back(
        std::make_shared<shared_model::proto::Transaction>(std::move(tx)));
  }

  auto tx_sequence_result =
      shared_model::interface::TransactionSequence::createTransactionSequence(
          txs, shared_model::validation::DefaultSignedTransactionsValidator());

  return framework::expected::val(tx_sequence_result).value().value;
}

/**
 * @given some user
 * @when sending sample AddAssetQuantity transactions to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status on that transactions,
 * all transactions are passed to proposal and does not appear in block
 */
TEST(PipelineIntegrationTest, SendTxSequence) {
  size_t tx_size = 5;
  const auto &tx_sequence = prepareTransactionSequence(tx_size);

  auto check_stateless_valid = [](auto &statuses) {
    for (const auto &status : statuses) {
      EXPECT_NO_THROW(boost::apply_visitor(
          framework::SpecifiedVisitor<
              shared_model::interface::StatelessValidTxResponse>(),
          status.get()));
    }
  };
  auto check_proposal = [&tx_size](auto &proposal) {
    ASSERT_EQ(proposal->transactions().size(), tx_size);
  };
  auto check_block = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  integration_framework::IntegrationTestFramework(
      tx_size)  // make all transactions to fit into a single proposal
      .setInitialState(kAdminKeypair)
      .sendTxSequence(tx_sequence, check_stateless_valid)
      .checkProposal(check_proposal)
      .checkBlock(check_block)
      .done();
}

/**
 * @give some user
 * @when sending transaction sequence with stateful invalid transactions to the
 * ledger using sendTxSequence await method
 * @then all transactions does not appear in the block
 */
TEST(PipelineIntegrationTest, SendTxSequenceAwait) {
  size_t tx_size = 5;
  const auto &tx_sequence = prepareTransactionSequence(tx_size);

  auto check_block = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  integration_framework::IntegrationTestFramework(
      tx_size)  // make all transactions to fit into a single proposal
      .setInitialState(kAdminKeypair)
      .sendTxSequenceAwait(tx_sequence, check_block)
      .done();
}
