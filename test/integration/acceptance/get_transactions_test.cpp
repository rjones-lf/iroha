/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/query_responses/transactions_response.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

class GetTransactions : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kGetMyTxs}) {
    auto new_perms = perms;
    new_perms.set(interface::permissions::Role::kSetQuorum);
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
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kReadAssets}))
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          dummy_tx,
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(dummy_tx.hash()), check);
}

/**
 * @given some user with only can_get_all_txs permission
 * @when query GetTransactions of existing transaction of the user
 * @then receive TransactionsResponse with the transaction hash
 */
TEST_F(GetTransactions, HaveGetAllTx) {
  auto dummy_tx = dummyTx();
  auto check = [&dummy_tx](auto &status) {
    ASSERT_NO_THROW({
      const auto &resp = boost::apply_visitor(
          framework::SpecifiedVisitor<interface::TransactionsResponse>(),
          status.get());
      ASSERT_EQ(resp.transactions().size(), 1);
      ASSERT_EQ(resp.transactions().front(), dummy_tx);
    });
  };

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({interface::permissions::Role::kGetAllTxs}))
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          dummy_tx,
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(dummy_tx.hash()), check);
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions of existing transaction of the user
 * @then receive TransactionsResponse with the transaction hash
 */
TEST_F(GetTransactions, HaveGetMyTx) {
  auto dummy_tx = dummyTx();
  auto check = [&dummy_tx](auto &status) {
    ASSERT_NO_THROW({
      const auto &resp = boost::apply_visitor(
          framework::SpecifiedVisitor<interface::TransactionsResponse>(),
          status.get());
      ASSERT_EQ(resp.transactions().size(), 1);
      ASSERT_EQ(resp.transactions().front(), dummy_tx);
    });
  };

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          dummy_tx,
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(dummy_tx.hash()), check);
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
                       crypto::DefaultCryptoAlgorithmType::generateKeypair())
                   .finish();

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendQuery(query, check);
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions with nonexistent hash
 * @then TransactionsResponse with no transactions
 */
TEST_F(GetTransactions, NonexistentHash) {
  auto check = [](auto &status) {
    ASSERT_NO_THROW({
      const auto &resp = boost::apply_visitor(
          framework::SpecifiedVisitor<interface::TransactionsResponse>(),
          status.get());
      ASSERT_EQ(resp.transactions().size(), 0);
    });
  };

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms(),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(crypto::Hash(std::string(
                     crypto::DefaultCryptoAlgorithmType::kHashLength, '0'))),
                 check);
}

/**
 * @given some user with can_get_my_txs
 * @when query GetTransactions of existing transaction of the other user
 * @then TransactionsResponse with no transactions
 */
TEST_F(GetTransactions, OtherUserTx) {
  auto check = [](auto &status) {
    ASSERT_NO_THROW({
      const auto &resp = boost::apply_visitor(
          framework::SpecifiedVisitor<interface::TransactionsResponse>(),
          status.get());
      ASSERT_EQ(resp.transactions().size(), 0);
    });
  };

  auto tx = makeUserWithPerms();
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          tx, [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(tx.hash()), check);
}
