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

#include "gtest/gtest.h"
#include "logger/logger.hpp"
#include "ametsuchi/impl/test_storage_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/model_hash_provider_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;

class TestStorageFixture : public AmetsuchiTest {

  void TearDown() override {
  }

};

Block getBlock() {
  HashProviderImpl hashProvider;
  Transaction txn;
  txn.creator_account_id = "admin1";
  AddPeer add_peer;
  add_peer.address = "192.168.0.0";
  txn.commands.push_back(std::make_shared<AddPeer>(add_peer));
  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = hashProvider.get_hash(block);
  block.hash = block1hash;
  block.txs_number = block.transactions.size();
  return block;
}

TEST_F(TestStorageFixture, TestingStorageWhenInsertBlock) {
  auto log = logger::testLog("TestStorage");
  log->info("Test case: create storage "
                "=> insert block "
                "=> assert that inserted");
  auto storage =
      TestStorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  ASSERT_EQ(0, wsv->getPeers().value().size());

  log->info("Try insert block");

  auto inserted = storage->insertBlock(getBlock());
  ASSERT_TRUE(inserted);

  log->info("Request ledger information");

  ASSERT_NE(0, wsv->getPeers().value().size());

  log->info("Drop ledger");

  storage->dropStorage();
}

TEST_F(TestStorageFixture, TestingStorageWhenDropAll) {
  auto logger = logger::testLog("TestStorage");
  logger->info("Test case: create storage "
                   "=> insert block "
                   "=> assert that written"
                   " => drop all "
                   "=> assert that all deleted ");

  auto log = logger::testLog("TestStorage");
  log->info("Test case: create storage "
                "=> insert block "
                "=> assert that inserted");
  auto storage =
      TestStorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  ASSERT_EQ(0, wsv->getPeers().value().size());

  log->info("Try insert block");

  auto inserted = storage->insertBlock(getBlock());
  ASSERT_TRUE(inserted);

  log->info("Request ledger information");

  ASSERT_NE(0, wsv->getPeers().value().size());

  log->info("Drop ledger");

  storage->dropStorage();

  ASSERT_EQ(0, wsv->getPeers().value().size());
  auto new_storage =
      TestStorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_EQ(0, wsv->getPeers().value().size());
  new_storage->dropStorage();
}
