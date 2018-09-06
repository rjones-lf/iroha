/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/integration_test_framework.hpp"

#include <gtest/gtest.h>

#include "builders/protobuf/transaction.hpp"
#include "framework/batch_helper.hpp"

const shared_model::crypto::Keypair keypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

/**
 * prepares signed transaction with CreateDomain command
 * @param domain_name name of the domain
 * @return Transaction with CreateDomain command
 */
auto createInvalidTransaction(std::string domain_name = "domain") {
  return shared_model::proto::TransactionBuilder()
      .createdTime(iroha::time::now())
      .quorum(1)
      .creatorAccountId("nonexistinguser@domain")
      .createDomain(domain_name, "user")
      .build()
      .signAndAddSignature(keypair)
      .finish();
}

/**
 * prepares transaction sequence
 * @param tx_size the size of transaction sequence
 * @return  transaction sequence
 */
auto prepareTransactionSequence(size_t tx_size) {
  shared_model::interface::types::SharedTxsCollectionType txs;

  for (size_t i = 0; i < tx_size; i++) {
    auto &&tx = createInvalidTransaction(std::string("domain")
                                               + std::to_string(i));
    txs.push_back(
        std::make_shared<shared_model::proto::Transaction>(std::move(tx)));
  }

  auto tx_sequence_result =
      shared_model::interface::TransactionSequence::createTransactionSequence(
          txs, shared_model::validation::DefaultSignedTransactionsValidator());

  return framework::expected::val(tx_sequence_result).value().value;
}

/**
 * @given set of stateful invalid transactions
 * @when all transactions are sent to iroha
 * @then verified proposal is empty and no block is committed
 */
TEST(EmptyBlockPipelineIntegrationTest, SendInvalidSequence) {
  size_t tx_size = 5;
  const auto &tx_sequence = prepareTransactionSequence(tx_size);

  auto check_block = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  integration_framework::IntegrationTestFramework(
      tx_size)  // make all transactions to fit into a single proposal
      .setInitialState(keypair)
      .sendTxSequence(tx_sequence, [](const auto &) {})
      .checkProposal([](const auto proposal) {})
      .checkVerifiedProposal(check_block);
}
