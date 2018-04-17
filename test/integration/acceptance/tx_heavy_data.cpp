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
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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
  auto makeUserWithPerms(
      const std::vector<std::string> &perms = {
          shared_model::permissions::role_perm_group.begin(),
          shared_model::permissions::role_perm_group.end()}) {
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
   * @return new string with passed quantity length
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
  const std::string kUserId =
      kUser + "@"s + IntegrationTestFramework::kDefaultDomain;
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given some user with all required permissions
 * @when send tx with addAccountDetail with big, but stateless invalid data
 * inside
 * @then transaction is passed
 */
TEST_F(HeavyTransactionTest, OneLargeTx) {
  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // "foo" transactions will not be passed because it has large size into
      // one field - 5Mb per one set
      .sendTx(complete(setAcountDetailTx("foo", generateData(5 * 1024 * 1024))),
              [](const auto &status) {
                ASSERT_TRUE(boost::apply_visitor(
                    shared_model::interface::SpecifiedVisitor<
                        shared_model::interface::StatelessFailedTxResponse>(),
                    status.get()));
              })
      .done();
}

/**
 * NOTE: test is disabled until fix of
 * https://soramitsu.atlassian.net/browse/IR-1205 will not be completed.
 * @given some user with all required permissions
 * @when send many txes with addAccountDetail with large data inside
 * @then transaction have been passed
 */
TEST_F(HeavyTransactionTest, DISABLED_ManyLargeTxes) {
  IntegrationTestFramework itf;

  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock([](auto &b) { ASSERT_EQ(b->transactions().size(), 1); });

  auto number_of_txes = 4u;
  for (auto i = 0u; i < number_of_txes; ++i) {
    itf.sendTx(complete(setAcountDetailTx("foo_" + std::to_string(i),
                                          generateData(2 * 1024 * 1024))));
  }
  itf.skipProposal()
      .checkBlock(
          [&](auto &b) { ASSERT_EQ(b->transactions().size(), number_of_txes); })
      .done();
}

/**
 * NOTE: test is disabled until fix of
 * https://soramitsu.atlassian.net/browse/IR-1205 will not be completed.
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
      // in itf tx build from large_tx_build will pass in Torii but in
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
 * https://soramitsu.atlassian.net/browse/IR-1205 will not be completed.
 * @given some user with all required permissions
 * AND max proposal size is 1.
 * @when send txes with addAccountDetail with large data inside.
 * AND transactions are passed stateful validation
 * @then query executed sucessfully
 */
TEST_F(HeavyTransactionTest, DISABLED_QueryLargeData) {
  auto number_of_times = 15u;
  auto size_of_data = 3 * 1024 * 1024u;
  auto data = generateData(size_of_data);

  auto name_generator = [](auto val) { return "foo_" + std::to_string(val); };

  auto query_checker = [&](auto &status) {
    auto response = *boost::apply_visitor(
        interface::SpecifiedVisitor<interface::AccountResponse>(),
        status.get());

    boost::property_tree::ptree root;
    boost::property_tree::read_json(response->account().jsonData(), root);
    auto user = root.get_child(kUserId);

    ASSERT_EQ(number_of_times, user.size());

    for (auto i = 0u; i < number_of_times; ++i) {
      ASSERT_EQ(data, user.get<std::string>(name_generator(i)));
    }
  };

  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair).sendTx(makeUserWithPerms());

  for (auto i = 0u; i < number_of_times; ++i) {
    itf.sendTx(complete(setAcountDetailTx(name_generator(i), data)))
        .skipProposal()
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  }

  // The query works fine only with ITF. It doesn't work in production version
  // of Iroha
  itf.sendQuery(complete(baseQuery().getAccount(kUserId)), query_checker)
      .done();
}
