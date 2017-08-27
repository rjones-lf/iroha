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
#include "ametsuchi/impl/storage_impl.hpp"
#include "common/byteutils.hpp"
#include "framework/test_subscriber.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/assign_master_key.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/model_hash_provider_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

TEST_F(AmetsuchiTest, GetBlocksCompletedWhenCalled) {
  // Commit block => get block => observable completed
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);

  Block block;
  block.height = 1;

  auto ms = storage->createMutableStorage();
  ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
  storage->commit(std::move(ms));

  auto completed_wrapper =
      make_test_subscriber<IsCompleted>(storage->getBlocks(1, 1));
  completed_wrapper.subscribe();
  ASSERT_TRUE(completed_wrapper.validate());
}

TEST_F(AmetsuchiTest, SampleTest) {
  HashProviderImpl hashProvider;

  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);

  Transaction txn;
  txn.creator_account_id = "admin1";
  CreateDomain createDomain;
  createDomain.domain_name = "ru";
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));
  CreateAccount createAccount;
  createAccount.account_name = "user1";
  createAccount.domain_id = "ru";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = hashProvider.get_hash(block);
  block.hash = block1hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &blk, auto &query, const auto &top_hash) {
      return true;
    });
    storage->commit(std::move(ms));
  }

  {
    auto account = storage->getAccount(createAccount.account_name + "@" +
                                       createAccount.domain_id);
    ASSERT_TRUE(account);
    ASSERT_EQ(account->account_id,
              createAccount.account_name + "@" + createAccount.domain_id);
    ASSERT_EQ(account->domain_name, createAccount.domain_id);
    ASSERT_EQ(account->master_key, createAccount.pubkey);
  }

  txn = Transaction();
  txn.creator_account_id = "admin2";
  createAccount = CreateAccount();
  createAccount.account_name = "user2";
  createAccount.domain_id = "ru";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));
  CreateAsset createAsset;
  createAsset.domain_id = "ru";
  createAsset.asset_name = "RUB";
  createAsset.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset));
  AddAssetQuantity addAssetQuantity;
  addAssetQuantity.asset_id = "RUB#ru";
  addAssetQuantity.account_id = "user1@ru";
  iroha::Amount asset_amount;
  asset_amount.int_part = 1;
  asset_amount.frac_part = 50;
  addAssetQuantity.amount = asset_amount;
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity));
  TransferAsset transferAsset;
  transferAsset.src_account_id = "user1@ru";
  transferAsset.dest_account_id = "user2@ru";
  transferAsset.asset_id = "RUB#ru";
  iroha::Amount transfer_amount;
  transfer_amount.int_part = 1;
  transfer_amount.frac_part = 0;
  transferAsset.amount = transfer_amount;
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 2;
  block.prev_hash = block1hash;
  auto block2hash = hashProvider.get_hash(block);
  block.hash = block2hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto asset1 = storage->getAccountAsset("user1@ru", "RUB#ru");
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, "user1@ru");
    ASSERT_EQ(asset1->asset_id, "RUB#ru");
    ASSERT_EQ(asset1->balance, 50);
    auto asset2 = storage->getAccountAsset("user2@ru", "RUB#ru");
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, "user2@ru");
    ASSERT_EQ(asset2->asset_id, "RUB#ru");
    ASSERT_EQ(asset2->balance, 100);
  }

  // Block store tests
  storage->getBlocks(1, 2).subscribe([block1hash, block2hash](auto eachBlock) {
    if (eachBlock.height == 1) {
      EXPECT_EQ(eachBlock.hash, block1hash);
    } else if (eachBlock.height == 2) {
      EXPECT_EQ(eachBlock.hash, block2hash);
    }
  });

  storage->getAccountTransactions("admin1").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 2); });
  storage->getAccountTransactions("admin2").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 4); });

  storage->getAccountAssetTransactions("user1@ru", "RUB#ru").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  storage->getAccountAssetTransactions("user2@ru", "RUB#ru").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
}

TEST_F(AmetsuchiTest, PeerTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);

  Transaction txn;
  AddPeer addPeer;
  addPeer.peer_key.at(0) = 1;
  addPeer.address = "192.168.0.1:50051";
  txn.commands.push_back(std::make_shared<AddPeer>(addPeer));

  Block block;
  block.transactions.push_back(txn);

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  auto peers = storage->getPeers();
  ASSERT_TRUE(peers);
  ASSERT_EQ(peers->size(), 1);
  ASSERT_EQ(peers->at(0).pubkey, addPeer.peer_key);
  ASSERT_EQ(peers->at(0).address, addPeer.address);
}

TEST_F(AmetsuchiTest, queryGetAccountAssetTransactionsTest) {
  HashProviderImpl hashProvider;

  auto storage = StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);

  const auto admin = "admin1";
  const auto domain = "domain";
  const auto user1name = "user1";
  const auto user2name = "user2";
  const auto user3name = "user3";
  const auto user1id = "user1@domain";
  const auto user2id = "user2@domain";
  const auto user3id = "user3@domain";
  const auto asset1name = "asset1";
  const auto asset2name = "asset2";
  const auto asset1id = "asset1#domain";
  const auto asset2id = "asset2#domain";

  // 1st tx
  Transaction txn;
  txn.creator_account_id = admin;
  CreateDomain createDomain;
  createDomain.domain_name = domain;
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));
  CreateAccount createAccount1;
  createAccount1.account_name = user1name;
  createAccount1.domain_id = domain;
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount1));
  CreateAccount createAccount2;
  createAccount2.account_name = user2name;
  createAccount2.domain_id = domain;
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount2));
  CreateAccount createAccount3;
  createAccount3.account_name = user3name;
  createAccount3.domain_id = domain;
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount3));
  CreateAsset createAsset1;
  createAsset1.domain_id = domain;
  createAsset1.asset_name = asset1name;
  createAsset1.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset1));
  CreateAsset createAsset2;
  createAsset2.domain_id = domain;
  createAsset2.asset_name = asset2name;
  createAsset2.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset2));
  AddAssetQuantity addAssetQuantity1;
  addAssetQuantity1.asset_id = asset1id;
  addAssetQuantity1.account_id = user1id;
  addAssetQuantity1.amount = iroha::Amount(3, 00);
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity1));
  AddAssetQuantity addAssetQuantity2;
  addAssetQuantity2.asset_id = asset2id;
  addAssetQuantity2.account_id = user2id;
  addAssetQuantity2.amount = iroha::Amount(2, 50);
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity2));

  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = hashProvider.get_hash(block);
  block.hash = block1hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &blk, auto &query, const auto &top_hash) {
      return true;
    });
    storage->commit(std::move(ms));
  }

  {
    auto account1 = storage->getAccount(user1id);
    ASSERT_TRUE(account1);
    ASSERT_EQ(account1->account_id, user1id);
    ASSERT_EQ(account1->domain_name, domain);
    auto account2 = storage->getAccount(user2id);
    ASSERT_TRUE(account2);
    ASSERT_EQ(account2->account_id, user2id);
    ASSERT_EQ(account2->domain_name, domain);
    auto account3 = storage->getAccount(user3id);
    ASSERT_TRUE(account3);
    ASSERT_EQ(account3->account_id, user3id);
    ASSERT_EQ(account3->domain_name, domain);

    auto asset1 = storage->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user1id);
    ASSERT_EQ(asset1->asset_id, asset1id);
    ASSERT_EQ(asset1->balance, 300);
    auto asset2 = storage->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user2id);
    ASSERT_EQ(asset2->asset_id, asset2id);
    ASSERT_EQ(asset2->balance, 250);
  }

  // 2th tx (user1 -> user2 # asset1)
  txn = Transaction();
  txn.creator_account_id = user1id;
  TransferAsset transferAsset;
  transferAsset.src_account_id = user1id;
  transferAsset.dest_account_id = user2id;
  transferAsset.asset_id = asset1id;
  transferAsset.amount = iroha::Amount(1, 20);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 2;
  block.prev_hash = block1hash;
  auto block2hash = hashProvider.get_hash(block);
  block.hash = block2hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto asset1 = storage->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user1id);
    ASSERT_EQ(asset1->asset_id, asset1id);
    ASSERT_EQ(asset1->balance, 180);
    auto asset2 = storage->getAccountAsset(user2id, asset1id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user2id);
    ASSERT_EQ(asset2->asset_id, asset1id);
    ASSERT_EQ(asset2->balance, 120);
  }

  // 3rd tx
  //   (user2 -> user3 # asset2)
  //   (user2 -> user1 # asset2)
  txn = Transaction();
  txn.creator_account_id = user2id;
  TransferAsset transferAsset1;
  transferAsset1.src_account_id = user2id;
  transferAsset1.dest_account_id = user3id;
  transferAsset1.asset_id = asset2id;
  transferAsset1.amount = iroha::Amount(1, 50);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset1));
  TransferAsset transferAsset2;
  transferAsset2.src_account_id = user2id;
  transferAsset2.dest_account_id = user1id;
  transferAsset2.asset_id = asset2id;
  transferAsset2.amount = iroha::Amount(0, 10);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset2));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 3;
  block.prev_hash = block2hash;
  auto block3hash = hashProvider.get_hash(block);
  block.hash = block3hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto asset1 = storage->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user2id);
    ASSERT_EQ(asset1->asset_id, asset2id);
    ASSERT_EQ(asset1->balance, 90);
    auto asset2 = storage->getAccountAsset(user3id, asset2id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user3id);
    ASSERT_EQ(asset2->asset_id, asset2id);
    ASSERT_EQ(asset2->balance, 150);
    auto asset3 = storage->getAccountAsset(user1id, asset2id);
    ASSERT_TRUE(asset3);
    ASSERT_EQ(asset3->account_id, user1id);
    ASSERT_EQ(asset3->asset_id, asset2id);
    ASSERT_EQ(asset3->balance, 10);
  }

  // Block store tests
  storage->getBlocks(1, 3).subscribe([block1hash, block2hash, block3hash](auto eachBlock) {
    if (eachBlock.height == 1) {
      EXPECT_EQ(eachBlock.hash, block1hash);
    } else if (eachBlock.height == 2) {
      EXPECT_EQ(eachBlock.hash, block2hash);
    } else if (eachBlock.height == 3) {
      EXPECT_EQ(eachBlock.hash, block3hash);
    }
  });

  storage->getAccountTransactions(admin).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 8); });
  storage->getAccountTransactions(user1id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  storage->getAccountTransactions(user2id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 2); });
  storage->getAccountTransactions(user3id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 0); });

  // (user1 -> user2 # asset1)
  // (user2 -> user3 # asset2)
  // (user2 -> user1 # asset2)
  storage->getAccountAssetTransactions(user1id, asset1id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  storage->getAccountAssetTransactions(user2id, asset1id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  storage->getAccountAssetTransactions(user3id, asset1id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 0); });
  storage->getAccountAssetTransactions(user1id, asset2id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  storage->getAccountAssetTransactions(user2id, asset2id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 2); });
  storage->getAccountAssetTransactions(user3id, asset2id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
}
