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
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/impl/storage_impl.hpp"
#include "crypto/hash.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/create_role.hpp"
#include "model/permissions.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;

/**
 * Fixture for yuna storage test. Creates two account, one containing age
 * information in json field, another has empty json information
 */
class YunaTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();
    storage =
        StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
    ASSERT_TRUE(storage);
    blocks = storage->getBlockQuery();
    wsv_query = storage->getWsvQuery();

    // First transaction in block1
    Transaction txn1_1;
    txn1_1.creator_account_id = "user1@test";

    CreateRole createRole;
    createRole.role_name = "user";
    createRole.permissions = {
        can_add_peer, can_create_asset, can_get_my_account};

    // Create domain ru
    txn1_1.commands.push_back(std::make_shared<CreateRole>(createRole));
    CreateDomain createDomain;
    createDomain.domain_id = "ru";
    createDomain.user_default_role = "user";
    txn1_1.commands.push_back(std::make_shared<CreateDomain>(createDomain));

    // Create account user1
    CreateAccount createAccount1;
    createAccount1.account_name = account_name1;
    createAccount1.domain_id = domain_id;
    createAccount1.json_data = account_data1;
    txn1_1.commands.push_back(std::make_shared<CreateAccount>(createAccount1));

    // Create account user2
    CreateAccount createAccount2;
    createAccount2.account_name = account_name2;
    createAccount2.domain_id = domain_id;
    createAccount2.json_data = "{}";
    txn1_1.commands.push_back(std::make_shared<CreateAccount>(createAccount2));

    Block block1;
    block1.height = 1;
    block1.transactions.push_back(txn1_1);
    block1.prev_hash.fill(0);
    auto block1hash = iroha::hash(block1);
    block1.hash = block1hash;
    block1.txs_number = block1.transactions.size();

    {
      auto ms = storage->createMutableStorage();
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
  std::string account_name1 = "user1";
  std::string account_data1 = R"({"age": "30"})";
  std::string account_name2 = "user2";
};

/**
 * @given storage with account containing json data
 * @when get account detail query is invoked
 * @then the requested information is returned
 */
TEST_F(YunaTest, GetAccountDetail) {
  auto account_id1 = account_name1 + "@" + domain_id;
  auto account = wsv_query->getAccount(account_id1);
  ASSERT_TRUE(account);
  ASSERT_EQ(account->account_id, account_id1);
  ASSERT_EQ(account->domain_id, domain_id);
  ASSERT_EQ(account->json_data, account_data1);

  auto age = wsv_query->getAccountDetail(account_id1, "age");
  ASSERT_TRUE(age);
  ASSERT_EQ(age.value(), "30");
}
