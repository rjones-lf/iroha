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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "main/raw_block_insertion.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"

#include <fstream>
#include <iostream>
#include <model/model_hash_provider_impl.hpp>
#include "model/converters/json_block_factory.hpp"
#include "model/converters/pb_command_factory.hpp"
#include "common/types.hpp"
#include "model/block.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/transaction.hpp"
#include "model/converters/json_common.hpp"

using ::testing::DefaultValue;
using ::testing::_;
using namespace iroha::main;
using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace iroha::model::converters;
using namespace iroha;
using namespace std;

MockMutableStorage *storage_mock = new MockMutableStorage();

std::unique_ptr<MutableStorage> createMutableStorageMock() {
  return unique_ptr<MockMutableStorage>(storage_mock);
}

Transaction getAddPeerTransaction(uint64_t create_time, std::string address) {
  Transaction transaction{};
  transaction.created_ts = create_time;
  Signature sign{};
  transaction.signatures = {sign};

  auto add_peer = std::make_shared<AddPeer>();
  add_peer->address = address;
  add_peer->peer_key = std::string(add_peer->peer_key.size(), '\0');

  transaction.commands = {add_peer};
  return transaction;
}

Transaction getTestCreateTransaction(uint64_t create_time) {
  Transaction transaction{};
  transaction.created_ts = create_time;
  Signature sign{};
  transaction.signatures = {sign};

  auto create_domain = std::make_shared<CreateDomain>();
  create_domain->domain_name = "test";

  auto create_asset = std::make_shared<CreateAsset>();
  create_asset->domain_id = "test";
  create_asset->asset_name = "coin";
  create_asset->precision = 2;

  auto create_admin = std::make_shared<CreateAccount>();
  create_admin->domain_id = "test";
  create_admin->account_name = "admin";

  auto create_acc = std::make_shared<CreateAccount>();
  create_acc->domain_id = "test";
  create_acc->account_name = "test";

  auto set_perm = std::make_shared<SetAccountPermissions>();
  set_perm->account_id = "admin@test";
  Account::Permissions permissions;
  permissions.can_transfer = true;
  permissions.read_all_accounts = true;
  permissions.issue_assets = true;
  permissions.set_permissions = true;
  set_perm->new_permissions = permissions;


  transaction.commands = {create_domain, create_asset, create_admin, create_acc,
                          set_perm};
  return transaction;
}

Block generateBlock() {
  Block block;
  block.created_ts =
      (ts64_t)chrono::system_clock::now().time_since_epoch().count();
  block.height = 1;
  std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
  block.txs_number = 4;

  auto start_port = 10001u;
  for (size_t i = start_port; i < start_port + block.txs_number; ++i) {
    block.transactions.push_back(getAddPeerTransaction(
        block.created_ts, "0.0.0.0:" + std::to_string(i)));
  }

  block.transactions.push_back(getTestCreateTransaction(block.created_ts));
  block.txs_number++;

  Signature sign{};
  block.sigs = {sign};
  block.hash = model::HashProviderImpl().get_hash(block);
  return block;
}

bool save_to_file(std::string json, std::string file_name) {
  // Save pubkey to file
  std::ofstream file(file_name);
  if (not file) {
    return false;
  }
  file << json;
  return true;
}

TEST(JsonRepr, JsonBlockReprGeneration) {
  cout << "----------| generate block => print |----------" << endl;

  JsonBlockFactory serializer;
  auto blob = serializer.serialize(generateBlock());
  auto json_block = jsonToString(blob);

  cout << json_block << endl;
  ASSERT_TRUE(save_to_file(json_block, "zero.block"));
}

TEST(BlockInsertionTest, BlockInsertionWhenParseBlock) {
  cout << "----------| block => string_repr(block)"
          " => parseBlock() |----------"
       << endl;

  shared_ptr<MockMutableFactory> factory = make_shared<MockMutableFactory>();
  BlockInserter inserter(factory);
  auto block = generateBlock();
  auto doc = JsonBlockFactory().serialize(block);
  auto str = jsonToString(doc);
  auto new_block = inserter.parseBlock(str);
  ASSERT_TRUE(new_block.has_value());
  ASSERT_EQ(block, new_block.value());
}

TEST(BlockInsertionTest, BlockInsertionWhenApplyToStorage) {
  cout << "----------| block => applyToLedger() |----------" << endl;

  shared_ptr<MockMutableFactory> factory = make_shared<MockMutableFactory>();
  DefaultValue<std::unique_ptr<MutableStorage>>::SetFactory(
      &createMutableStorageMock);

  EXPECT_CALL(*factory, createMutableStorage()).Times(1);
  EXPECT_CALL(*factory, commit_(_)).Times(1);
  EXPECT_CALL(*storage_mock, apply(_, _)).Times(1);

  BlockInserter inserter(factory);
  auto block = generateBlock();
  vector<Block> blocks{block};

  inserter.applyToLedger(blocks);
}
