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
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"

constexpr auto kUser = "user@test";
constexpr auto kAsset = "asset#domain";
const auto kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

/**
 * @given ITF instance with Iroha
 * @when existing ITF instance was not gracefully shutdown
 * @then following ITF instantiation should not cause any errors
 */
TEST(RegressionTest, SequentialInitialization) {
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId(kUser)
                .addAssetQuantity(kUser, kAsset, "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair());

  auto checkStatelessValid = [](auto &status) {
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::
            SpecifiedVisitor<shared_model::interface::
                                 StatelessValidTxResponse>(),
        status.get()));
  };
  auto checkProposal = [](auto &proposal) {
    ASSERT_EQ(proposal->transactions().size(), 1);
  };
  auto checkBlock = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  {
    integration_framework::IntegrationTestFramework(1, [](auto &) {})
        .setInitialState(kAdminKeypair)
        .sendTx(tx, checkStatelessValid)
        .skipProposal()
        .skipBlock();
  }
  {
    integration_framework::IntegrationTestFramework(1)
        .setInitialState(kAdminKeypair)
        .sendTx(tx, checkStatelessValid)
        .checkProposal(checkProposal)
        .checkBlock(checkBlock)
        .done();
  }
}

/**
 * @given ITF instance
 * @when instance is shutdown without blocks erase
 * @then another ITF instance can restore WSV from blockstore
 */
TEST(RegressionTest, StateRecovery) {
  auto userKeypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId("admin@test")
                .createAccount("user", "test", userKeypair.publicKey())
                .addAssetQuantity("admin@test", "coin#test", "133.0")
                .transferAsset(
                    "admin@test", "user@test", "coin#test", "descrs", "97.8")
                .quorum(1)
                .build()
                .signAndAddSignature(kAdminKeypair);
  auto hash = tx.hash();
  auto makeQuery = [&hash](int query_counter, auto kAdminKeypair) {
    return shared_model::proto::QueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId("admin@test")
        .queryCounter(query_counter)
        .getTransactions(std::vector<shared_model::crypto::Hash>{hash})
        .build()
        .signAndAddSignature(kAdminKeypair);
  };
  auto checkOne = [](auto &res) { ASSERT_EQ(res->transactions().size(), 1); };
  auto checkQuery = [&tx](auto &status) {
    auto resp = boost::apply_visitor(
        shared_model::interface::
            SpecifiedVisitor<shared_model::interface::TransactionsResponse>(),
        status.get());
    ASSERT_TRUE(resp);
    ASSERT_EQ(resp.value().transactions().size(), 1);
    ASSERT_EQ(*resp.value().transactions()[0].operator->(), tx);
  };
  auto path =
      (boost::filesystem::temp_directory_path() / "iroha-state-recovery-test")
          .string();
  {
    integration_framework::IntegrationTestFramework(
        1, [](auto &) {}, false, path)
        .setInitialState(kAdminKeypair)
        .sendTx(tx)
        .checkProposal(checkOne)
        .checkBlock(checkOne)
        .sendQuery(makeQuery(1, kAdminKeypair), checkQuery);
  }
  {
    integration_framework::IntegrationTestFramework(
        1, [](auto &itf) { itf.done(); }, false, path)
        .recoverState(kAdminKeypair)
        .sendQuery(makeQuery(2, kAdminKeypair), checkQuery)
        .done();
  }
}

/**
 * @given ITF instance with Iroha
 * @when done method is called twice
 * @then no errors are caused as the result
 */
TEST(RegressionTest, DoubleCallOfDone) {
  integration_framework::IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair).done();
  itf.done();
}

/**
 * @given non initialized ITF instance
 * @when done method is called inside destructor
 * @then no exceptions are risen
 */
TEST(RegressionTest, DestructionOfNonInitializedItf) {
  integration_framework::IntegrationTestFramework itf(
      1, [](auto &itf) { itf.done(); });
}
