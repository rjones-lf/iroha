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
#include "common/files.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"

constexpr auto kAdmin = "user@test";
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
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAsset, "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();

  auto checkStatelessValid = [](auto &status) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::StatelessValidTxResponse>(),
        status.get()));
  };
  auto checkProposal = [](auto &proposal) {
    ASSERT_EQ(proposal->transactions().size(), 1);
  };

  const std::string dbname = "dbseqinit";
  {
    integration_framework::IntegrationTestFramework(1, dbname, [](auto &) {})
        .setInitialState(kAdminKeypair)
        .sendTx(tx, checkStatelessValid)
        .skipProposal()
        .checkVerifiedProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 0);
        });
  }
  {
    integration_framework::IntegrationTestFramework(1, dbname)
        .setInitialState(kAdminKeypair)
        .sendTx(tx, checkStatelessValid)
        .checkProposal(checkProposal)
        .checkVerifiedProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 0);
        })
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
                .addAssetQuantity("coin#test", "133.0")
                .transferAsset(
                    "admin@test", "user@test", "coin#test", "descrs", "97.8")
                .quorum(1)
                .build()
                .signAndAddSignature(kAdminKeypair)
                .finish();
  auto hash = tx.hash();
  auto makeQuery = [&hash](int query_counter, auto kAdminKeypair) {
    return shared_model::proto::QueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId("admin@test")
        .queryCounter(query_counter)
        .getTransactions(std::vector<shared_model::crypto::Hash>{hash})
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  };
  auto checkOne = [](auto &res) { ASSERT_EQ(res->transactions().size(), 1); };
  auto checkQuery = [&tx](auto &status) {
    ASSERT_NO_THROW({
      const auto &resp = boost::apply_visitor(
          framework::SpecifiedVisitor<
              shared_model::interface::TransactionsResponse>(),
          status.get());
      ASSERT_EQ(resp.transactions().size(), 1);
      ASSERT_EQ(resp.transactions().front(), tx);
    });
  };
  auto path =
      (boost::filesystem::temp_directory_path() / "iroha-state-recovery-test")
          .string();
  const std::string dbname = "dbstatereq";

  // Cleanup blockstore directory, because it may contain blocks from previous
  // test launch if ITF was failed for some reason. If there are some blocks,
  // then checkProposal will fail with "missed proposal" error, because of
  // incorrect calculation of chain height.
  iroha::remove_dir_contents(path);

  {
    integration_framework::IntegrationTestFramework(
        1,
        dbname,
        [](auto &) {},
        false,
        path)
        .setInitialState(kAdminKeypair)
        .sendTx(tx)
        .checkProposal(checkOne)
        .checkBlock(checkOne)
        .sendQuery(makeQuery(1, kAdminKeypair), checkQuery);
  }
  {
    integration_framework::IntegrationTestFramework(
        1,
        dbname,
        [](auto &itf) { itf.done(); },
        false,
        path)
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
      1, {}, [](auto &itf) { itf.done(); });
}
