/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "utils/query_error_response_visitor.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

class GetTransactions : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             shared_model::permissions::can_get_my_txs}) {
    auto new_perms = perms;
    new_perms.push_back(shared_model::permissions::can_set_quorum);
    return AcceptanceFixture::makeUserWithPerms(kNewRole, new_perms);
  }

  /**
   * Valid transaction that user can execute.
   * @return built tx and a hash of its payload
   * Note: It should affect the ledger minimally
   */
  auto dummyTx() {
    return complete(AcceptanceFixture::baseTx().setAccountQuorum(kUserId, 1));
  }

  /**
   * Creates valid GetTransactions query of current user
   * @param hash of the tx for querying
   * @return built query
   */
  auto makeQuery(const crypto::Hash &hash) {
    return complete(baseQry().queryCounter(1).getTransactions(
        std::vector<crypto::Hash>{hash}));
  }

  const std::string kNewRole = "rl";
};

/**
 * @given some user without can_get_{my,all}_txs permissions
 * @when query GetTransactions of existing transaction of the user
 * @then stateful validation fail returned
 */
TEST_F(GetTransactions, HaveNoGetPerms) {
  auto check = [](auto &status) {
    ASSERT_TRUE(
        boost::apply_visitor(interface::QueryErrorResponseChecker<
                                 interface::StatefulFailedErrorResponse>(),
                             status.get()));
  };

  auto dummy_tx = dummyTx();
  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_read_assets}))
      .sendTx(dummy_tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendQuery(makeQuery(dummy_tx.hash()), check)
      .done();
}

/**
 * @given some user with only can_get_all_txs permission
 * @when query GetTransactions of existing transaction of the user
 * @then receive TransactionsResponse with the transaction hash
 */
TEST_F(GetTransactions, HaveGetAllTx) {
  auto dummy_tx = dummyTx();
  auto check = [&dummy_tx](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value().transactions().size(), 1);
    ASSERT_EQ(*resp.value().transactions()[0].operator->(), dummy_tx);
  };

  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({shared_model::permissions::can_get_all_txs}))
      .sendTx(dummy_tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendQuery(makeQuery(dummy_tx.hash()), check)
      .done();
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions of existing transaction of the user
 * @then receive TransactionsResponse with the transaction hash
 */
TEST_F(GetTransactions, HaveGetMyTx) {
  auto dummy_tx = dummyTx();
  auto check = [&dummy_tx](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value().transactions().size(), 1);
    ASSERT_EQ(*resp.value().transactions()[0].operator->(), dummy_tx);
  };

  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(dummy_tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendQuery(makeQuery(dummy_tx.hash()), check)
      .done();
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions of existing transaction of the user, but with
 * invalid signatures
 * @then receive StatefullErrorResponse
 */
TEST_F(GetTransactions, InvalidSignatures) {
  auto dummy_tx = dummyTx();
  auto check = [](auto &status) {
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::QueryErrorResponseChecker<
            shared_model::interface::StatefulFailedErrorResponse>(),
        status.get()));
  };

  auto query = baseQry()
                   .queryCounter(1)
                   .getTransactions(std::vector<crypto::Hash>{dummy_tx.hash()})
                   .build()
                   .signAndAddSignature(
                       crypto::DefaultCryptoAlgorithmType::generateKeypair());

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipBlock()
      .sendQuery(query, check)
      .done();
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions with nonexistent hash
 * @then TransactionsResponse with no transactions
 */
TEST_F(GetTransactions, NonexistentHash) {
  auto check = [](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value().transactions().size(), 0);
  };

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(crypto::Hash(std::string(32, '0'))), check)
      .done();
}

/**
 * @given some user with can_get_my_txs
 * @when query GetTransactions of existing transaction of the other user
 * @then TransactionsResponse with no transactions
 */
TEST_F(GetTransactions, OtherUserTx) {
  auto check = [](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value().transactions().size(), 0);
  };

  auto tx = makeUserWithPerms();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(tx.hash()), check)
      .done();
}
