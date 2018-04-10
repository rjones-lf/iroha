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
#include "datetime/time.hpp"
#include "framework/base_tx.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class HeavyTransactionTest : public ::testing::Test {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::vector<std::string> &perms = {
                             iroha::model::can_set_my_account_detail}) {
    return framework::createUserWithPerms(
               kUser, kUserKeypair.publicKey(), "role"s, perms)
        .build()
        .signAndAddSignature(kAdminKeypair);
  }

  /**
   * Create valid base pre-built transaction
   * @return pre-built tx
   */
  auto baseTx() {
    return TestUnsignedTransactionBuilder()
        .txCounter(1)
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now());
  }

  /**
   * Generate stub of transaction for setting data to default user account
   * @param key - key for the insertion
   * @param value - data that will be attached
   * @return generated builder, without signature
   */
  auto setAcountDetailTx(const std::string &key, const std::string &value) {
    return baseTx().setAccountDetail(kUserId, key, value);
  }

  /**
   * Util method for stub data generation
   * @param quantity - number of bytes
   * @return new string with passed quantity lenght
   */
  static auto generateData(size_t quantity) {
    return std::string(quantity, 'F');
  }

  /**
   * Sign pre-built object
   * @param builder is a pre-built signable object
   * @return completed object
   */
  template <typename Builder>
  auto complete(Builder builder) {
    return builder.build().signAndAddSignature(kUserKeypair);
  }

  /**
   * Create valid basis of pre-built query
   * @return query stub with counter, creator and time
   */
  auto baseQuery() {
    return TestUnsignedQueryBuilder()
        .queryCounter(1)
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now());
  }

  const std::string kUser = "user"s;
  const std::string kUserId = kUser + "@test";
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with all required permissions
 * @when send tx with addAccountDetail with large data inside
 * @then transaction is passed
 */
TEST_F(HeavyTransactionTest, OneLargeTx) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      // "foo" transactions will be not passed because it has large size into
      // one field - 5Mb per one set
      .sendTx(complete(setAcountDetailTx("foo", generateData(5 * 1024 * 1024))))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * NOTE: test is disabled until fix of
 * https://soramitsu.atlassian.net/browse/IR-1205 will be not completed.
 * @given some user with all required permissions
 * @when send many txes with addAccountDetail with large data inside
 * @then transaction have been passed
 */
TEST_F(HeavyTransactionTest, DISABLED_ManyLargeTxes) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .sendTx(
          complete(setAcountDetailTx("foo_1", generateData(2 * 1024 * 1024))))
      .sendTx(
          complete(setAcountDetailTx("foo_2", generateData(2 * 1024 * 1024))))
      .sendTx(
          complete(setAcountDetailTx("foo_3", generateData(2 * 1024 * 1024))))
      .sendTx(
          complete(setAcountDetailTx("foo_4", generateData(2 * 1024 * 1024))))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 4); })
      .done();
}

/**
 * NOTE: test is disabled until fix of
 * https://soramitsu.atlassian.net/browse/IR-1205 will be not completed.
 * @given some user with all required permissions
 * @when send tx with many addAccountDetails with large data inside
 * @then transaction is passed
 */
TEST_F(HeavyTransactionTest, DISABLED_VeryLargeTxWithManyCommands) {
  auto big_data = generateData(3 * 1024 * 1024);
  auto large_tx_builder = setAcountDetailTx("foo_1", big_data)
                              .setAccountDetail(kUserId, "foo_2", big_data)
                              .setAccountDetail(kUserId, "foo_3", big_data);

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      // in itf tx build from large_tx_build will be pass Torii but in
      // production the transaction will be failed before stateless validation
      // because of size.
      .sendTx(complete(large_tx_builder))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * NOTE: test is disabled until fix of
 * https://soramitsu.atlassian.net/browse/IR-1205 will be not completed.
 * @given some user with all required permissions
 * AND itf that hanlde only 1 tx in OS.
 * @when send txes with addAccountDetail with large data inside.
 * AND transactions are passed steful validation
 * @then transactions are passed
 * AND query for getting is done
 */
TEST_F(HeavyTransactionTest, DISABLED_QueryLargeData) {
  auto query_checker = [](auto &status) {
    boost::apply_visitor(
        interface::SpecifiedVisitor<interface::AccountDetailResponse>(),
        status.get());
  };

  auto data = generateData(2 * 1024 * 1024);

  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair).sendTx(makeUserWithPerms());

  for (auto i = 0; i < 5; i++) {
    itf.sendTx(complete(setAcountDetailTx("foo_1", data)))
        .skipProposal()
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  }

  itf.sendQuery(complete(baseQuery().getAccountDetail(kUserId)), query_checker)
      .done();
}
