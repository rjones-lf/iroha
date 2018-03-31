/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/storage_impl.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "model/block.hpp"
#include "validators/permissions.hpp"
#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

// TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
// shared_model https://soramitsu.atlassian.net/browse/IR-887
#include "backend/protobuf/from_old_model.hpp"

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
    auto storageResult = StorageImpl::create(block_store_path, pgopt_);
    storageResult.match(
        [&](iroha::expected::Value<std::shared_ptr<StorageImpl>> &_storage) {
          storage = _storage.value;
        },
        [](iroha::expected::Error<std::string> &error) {
          FAIL() << "StorageImpl: " << error.error;
        });
    ASSERT_TRUE(storage);
    blocks = storage->getBlockQuery();
    wsv_query = storage->getWsvQuery();

    // First transaction in block1
    iroha::model::Transaction txn1_1;
    txn1_1.creator_account_id = "userone@ru";

    iroha::model::CreateRole createRole;
    createRole.role_name = "user";
    createRole.permissions = {iroha::model::can_add_peer,
                              iroha::model::can_create_asset,
                              iroha::model::can_get_my_account};

    // Create domain ru
    txn1_1.commands.push_back(
        std::make_shared<iroha::model::CreateRole>(createRole));
    iroha::model::CreateDomain createDomain;
    createDomain.domain_id = "ru";
    createDomain.user_default_role = "user";
    txn1_1.commands.push_back(
        std::make_shared<iroha::model::CreateDomain>(createDomain));

    // Create account user1
    iroha::model::CreateAccount createAccount1;
    createAccount1.account_name = account_name1;
    createAccount1.domain_id = domain_id;
    txn1_1.commands.push_back(
        std::make_shared<iroha::model::CreateAccount>(createAccount1));

    // Create account user2
    iroha::model::CreateAccount createAccount2;
    createAccount2.account_name = account_name2;
    createAccount2.domain_id = domain_id;
    txn1_1.commands.push_back(
        std::make_shared<iroha::model::CreateAccount>(createAccount2));

    // Set age for user2
    iroha::model::SetAccountDetail setAccount2Age;
    setAccount2Age.account_id = account_name2 + "@" + domain_id;
    setAccount2Age.key = "age";
    setAccount2Age.value = "24";
    txn1_1.commands.push_back(
        std::make_shared<iroha::model::SetAccountDetail>(setAccount2Age));

    iroha::model::Block old_block1;
    old_block1.height = 1;
    old_block1.transactions.push_back(txn1_1);
    old_block1.prev_hash.fill(0);
    auto block1hash = iroha::hash(old_block1);
    old_block1.hash = block1hash;
    old_block1.txs_number = old_block1.transactions.size();

    {
      std::unique_ptr<MutableStorage> ms;
      auto storageResult = storage->createMutableStorage();
      storageResult.match(
          [&](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                  &_storage) { ms = std::move(_storage.value); },
          [](iroha::expected::Error<std::string> &error) {
            FAIL() << "MutableStorage: " << error.error;
          });
      // TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
      // shared_model https://soramitsu.atlassian.net/browse/IR-887
      auto block1 = shared_model::proto::from_old(old_block1);
      ms->apply(block1, [](const auto &blk, auto &query, const auto &top_hash) {
        return true;
      });
      storage->commit(std::move(ms));
    }
  }

  std::shared_ptr<StorageImpl> storage;
  std::shared_ptr<BlockQuery> blocks;
  std::shared_ptr<WsvQuery> wsv_query;

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
  auto ss =
      std::istringstream(wsv_query->getAccountDetail(account_id1).value());

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
  auto ss =
      std::istringstream(wsv_query->getAccountDetail(account_id2).value());

  boost::property_tree::ptree root;
  boost::property_tree::read_json(ss, root);

  auto record = root.get_child(account_id1);
  ASSERT_EQ(record.size(), 1);
  ASSERT_EQ(record.front().first, "age");
  ASSERT_EQ(record.front().second.data(), "24");
}
