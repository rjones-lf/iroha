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

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/storage_impl.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "interfaces/permissions.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace iroha::ametsuchi;

/**
 * Fixture for kv storage test. Creates two accounts: one has predefined json
 * information in json field, another one has json information filled using set
 * account detail method
 */
class KVTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();
    std::string empty_key(32, '0');
    // transaction for block 1
    auto txn =
        TestTransactionBuilder()
            .creatorAccountId("userone@ru")
            .createRole(
                "user",
                {shared_model::interface::permissions::Role::kAddPeer,
                 shared_model::interface::permissions::Role::kCreateAsset,
                 shared_model::interface::permissions::Role::kGetMyAccount})
            .createDomain("ru", "user")
            .createAccount(account_name1,
                           domain_id,
                           shared_model::crypto::PublicKey(empty_key))
            .createAccount(account_name2,
                           domain_id,
                           shared_model::crypto::PublicKey(empty_key))
            .setAccountDetail(account_name2 + "@" + domain_id, "age", "24")
            .build();
    auto block1 =
        TestBlockBuilder()
            .height(1)
            .prevHash(shared_model::crypto::Hash(empty_key))
            .transactions(std::vector<shared_model::proto::Transaction>{txn})
            .build();

    {
      std::unique_ptr<MutableStorage> ms;
      auto storageResult = storage->createMutableStorage();
      storageResult.match(
          [&](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                  &_storage) { ms = std::move(_storage.value); },
          [](iroha::expected::Error<std::string> &error) {
            FAIL() << "MutableStorage: " << error.error;
          });

      ms->apply(block1);
      storage->commit(std::move(ms));
    }
  }

  void TearDown() override {
    sql->close();
    AmetsuchiTest::TearDown();
  }

  std::string domain_id = "ru";
  std::string account_name1 = "userone";
  std::string account_name2 = "usertwo";
};

/**
 * @given no details is set in account1
 * @when detail of account1 is queried using GetAccountDetail
 * @then nullopt is returned
 */
TEST_F(KVTest, GetNonexistingUserDetail) {
  auto account_id1 = account_name1 + "@" + domain_id;
  auto ss = std::istringstream(
      storage->getWsvQuery()->getAccountDetail(account_id1).value());

  boost::property_tree::ptree root;
  boost::property_tree::read_json(ss, root);
  ASSERT_TRUE(root.empty());
}

/**
 * @given storage with account containing age inserted using SetAccountDetail
 * @when get account detail is invoked
 * @then correct age of user2 is returned
 */
TEST_F(KVTest, SetAccountDetail) {
  auto account_id1 = account_name1 + "@" + domain_id;
  auto account_id2 = account_name2 + "@" + domain_id;
  auto ss = std::istringstream(
      storage->getWsvQuery()->getAccountDetail(account_id2).value());

  boost::property_tree::ptree root;
  boost::property_tree::read_json(ss, root);

  auto record = root.get_child(account_id1);
  ASSERT_EQ(record.size(), 1);
  ASSERT_EQ(record.front().first, "age");
  ASSERT_EQ(record.front().second.data(), "24");
}
