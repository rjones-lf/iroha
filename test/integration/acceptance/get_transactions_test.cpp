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
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/base_tx.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "model/permissions.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class GetTransactions : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms) {
    auto new_perms = perms;
    new_perms.push_back(iroha::model::can_set_quorum);
    return framework::createUserWithPerms(
               kUser, kUserKeypair.publicKey(), kNewRole, new_perms)
        .build()
        .signAndAddSignature(kAdminKeypair);
  }

  /**
   * Valid transaction that user can execute.
   * @return built tx and a hash of its payload
   * Note: It should affect the ledger minimally
   */
  auto dummyTx() {
    return shared_model::proto::TransactionBuilder()
        .setAccountQuorum(kUserId, 1)
        .txCounter(1)
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now())
        .build()
        .signAndAddSignature(kUserKeypair);
  }

  /**
   * Creates valid GetTransactions query of current user
   * @param hash of the tx for querying
   * @return built query
   */
  auto makeQuery(const crypto::Hash &hash) {
    return proto::QueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId(kUserId)
        .queryCounter(1)
        .getTransactions(std::vector<crypto::Hash>{hash})
        .build()
        .signAndAddSignature(kUserKeypair);
  }

  const std::string kUser = "user"s;
  const std::string kNewRole = "rl"s;
  const std::string kUserId = kUser + "@test";
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
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
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_read_assets}))
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
    ASSERT_EQ(resp.value()->transactions().size(), 1);
    ASSERT_EQ(*resp.value()->transactions()[0].operator->(), dummy_tx);
  };

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_get_all_txs}))
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
    ASSERT_EQ(resp.value()->transactions().size(), 1);
    ASSERT_EQ(*resp.value()->transactions()[0].operator->(), dummy_tx);
  };

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_get_my_txs}))
      .sendTx(dummy_tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 2); })
      .sendQuery(makeQuery(dummy_tx.hash()), check)
      .done();
}

/**
 * @given some user with only can_get_my_txs permission
 * @when query GetTransactions with inexistent hash
 * @then TransactionsResponse with no transactions
 */
TEST_F(GetTransactions, InexistentHash) {
  auto check = [](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value()->transactions().size(), 0);
  };

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({iroha::model::can_get_my_txs}))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(crypto::Hash(std::string(32, '0'))), check)
      .done();
}

/**
 * @given some user with can_get_my_txs
 * @when query GetTransactions of existing transaction of the other user
 * @then TransactionsResponse with no transactions
 * TODO(@l4l) 02/01/18 Should be enabled after resolving IR-1039
 */
TEST_F(GetTransactions, DISABLED_OtherUserTx) {
  auto check = [](auto &status) {
    auto resp = boost::apply_visitor(
        interface::SpecifiedVisitor<interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value()->transactions().size(), 0);
  };

  auto tx = makeUserWithPerms({iroha::model::can_get_my_txs});
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(tx)
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(tx.hash()), check)
      .done();
}
