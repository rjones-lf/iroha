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
#include <model/commands/create_role.hpp>
#include "ametsuchi/impl/storage_impl.hpp"
#include "common/byteutils.hpp"
#include "crypto/hash.hpp"
#include "framework/test_subscriber.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/converters/pb_block_factory.hpp"
#include "model/permissions.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

TEST_F(AmetsuchiTest, GetBlocksCompletedWhenCalled) {
  // Commit block => get block => observable completed
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto blocks = storage->getBlockQuery();

  Block block;
  block.height = 1;

  auto ms = storage->createMutableStorage();
  ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
  storage->commit(std::move(ms));

  auto completed_wrapper =
      make_test_subscriber<IsCompleted>(blocks->getBlocks(1, 1));
  completed_wrapper.subscribe();
  ASSERT_TRUE(completed_wrapper.validate());
}

TEST_F(AmetsuchiTest, SampleTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();
  CreateRole createRole;
  createRole.role_name = "user";
  createRole.permissions = {can_add_peer, can_create_asset, can_get_my_account};

  // Tx 1
  Transaction txn;
  txn.creator_account_id = "admin1";

  // Create domain ru
  txn.commands.push_back(std::make_shared<CreateRole>(createRole));
  CreateDomain createDomain;
  createDomain.domain_id = "ru";
  createDomain.user_default_role = "user";
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));

  // Create account user1
  CreateAccount createAccount;
  createAccount.account_name = "user1";
  createAccount.domain_id = "ru";
  createAccount.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  // Compose block
  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = iroha::hash(block);
  block.hash = block1hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &blk, auto &query, const auto &top_hash) {
      return true;
    });
    storage->commit(std::move(ms));
  }

  {
    auto account = wsv->getAccount(createAccount.account_name + "@"
                                   + createAccount.domain_id);
    ASSERT_TRUE(account);
    ASSERT_EQ(account->account_id,
              createAccount.account_name + "@" + createAccount.domain_id);
    ASSERT_EQ(account->domain_id, createAccount.domain_id);
  }

  // Tx 2
  txn = Transaction();
  txn.creator_account_id = "admin2";

  // Create account user2
  createAccount = CreateAccount();
  createAccount.account_name = "user2";
  createAccount.domain_id = "ru";
  createAccount.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  // Create asset RUB#ru
  CreateAsset createAsset;
  createAsset.domain_id = "ru";
  createAsset.asset_name = "RUB";
  createAsset.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset));

  // Add RUB#ru to user1
  AddAssetQuantity addAssetQuantity;
  addAssetQuantity.asset_id = "RUB#ru";
  addAssetQuantity.account_id = "user1@ru";
  iroha::Amount asset_amount(150, 2);
  addAssetQuantity.amount = asset_amount;
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity));

  // Transfer asset from user 1
  TransferAsset transferAsset;
  transferAsset.src_account_id = "user1@ru";
  transferAsset.dest_account_id = "user2@ru";
  transferAsset.asset_id = "RUB#ru";
  transferAsset.description = "test transfer";
  iroha::Amount transfer_amount(100, 2);
  transferAsset.amount = transfer_amount;
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset));

  // Compose block
  block = Block();
  block.transactions.push_back(txn);
  block.height = 2;
  block.prev_hash = block1hash;
  auto block2hash = iroha::hash(block);
  block.hash = block2hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto asset1 = wsv->getAccountAsset("user1@ru", "RUB#ru");
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, "user1@ru");
    ASSERT_EQ(asset1->asset_id, "RUB#ru");
    ASSERT_EQ(asset1->balance, iroha::Amount(50, 2));

    auto asset2 = wsv->getAccountAsset("user2@ru", "RUB#ru");
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, "user2@ru");
    ASSERT_EQ(asset2->asset_id, "RUB#ru");
    ASSERT_EQ(asset2->balance, iroha::Amount(100, 2));
  }

  // Block store tests
  blocks->getBlocks(1, 2).subscribe([block1hash, block2hash](auto eachBlock) {
    if (eachBlock.height == 1) {
      EXPECT_EQ(eachBlock.hash, block1hash);
    } else if (eachBlock.height == 2) {
      EXPECT_EQ(eachBlock.hash, block2hash);
    }
  });

  blocks->getAccountTransactions("admin1").subscribe([](auto tx) {
    EXPECT_EQ(tx.creator_account_id, "admin1");
    EXPECT_EQ(tx.commands.size(), 3);
  });
  blocks->getAccountTransactions("admin2").subscribe([](auto tx) {
    EXPECT_EQ(tx.creator_account_id, "admin2");
    EXPECT_EQ(tx.commands.size(), 4);
  });

  // request for non-existing user
  auto number_of_calls = 0;
  auto getAccountsTxWrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions("non_existing_user"), 0);
  getAccountsTxWrapper.subscribe(
      [&number_of_calls](auto val) { number_of_calls++; });

  ASSERT_TRUE(getAccountsTxWrapper.validate());
  ASSERT_EQ(number_of_calls, 0);

  blocks->getAccountAssetTransactions("user1@ru", "RUB#ru")
      .subscribe([](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  blocks->getAccountAssetTransactions("user2@ru", "RUB#ru")
      .subscribe([](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });

  // request for non-existing asset
  number_of_calls = 0;
  auto getAccountAssetTxWrapper = make_test_subscriber<CallExact>(
      blocks->getAccountAssetTransactions("non_existing_user",
                                          "non_existing_asset"),
      0);
  getAccountAssetTxWrapper.subscribe(
      [&number_of_calls](auto val) { number_of_calls++; });

  ASSERT_TRUE(getAccountAssetTxWrapper.validate());
  ASSERT_EQ(number_of_calls, 0);
}

TEST_F(AmetsuchiTest, PeerTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();

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

  auto peers = wsv->getPeers();
  ASSERT_TRUE(peers);
  ASSERT_EQ(peers->size(), 1);
  ASSERT_EQ(peers->at(0).pubkey, addPeer.peer_key);
  ASSERT_EQ(peers->at(0).address, addPeer.address);
}

TEST_F(AmetsuchiTest, queryGetAccountAssetTransactionsTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();

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

  CreateRole createRole;
  createRole.role_name = "user";
  createRole.permissions = {can_add_peer, can_create_asset, can_get_my_account};

  // 1st tx
  Transaction txn;
  txn.creator_account_id = admin;

  // Create domain
  CreateDomain createDomain;
  txn.commands.push_back(std::make_shared<CreateRole>(createRole));
  createDomain.domain_id = domain;
  createDomain.user_default_role = "user";
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));

  // Create account 1
  CreateAccount createAccount1;
  createAccount1.account_name = user1name;
  createAccount1.domain_id = domain;
  createAccount1.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount1));

  // Create account 2
  CreateAccount createAccount2;
  createAccount2.account_name = user2name;
  createAccount2.domain_id = domain;
  createAccount2.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount2));

  // Create account 3
  CreateAccount createAccount3;
  createAccount3.account_name = user3name;
  createAccount3.domain_id = domain;
  createAccount3.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount3));

  // Create asset 1
  CreateAsset createAsset1;
  createAsset1.domain_id = domain;
  createAsset1.asset_name = asset1name;
  createAsset1.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset1));

  // Create asset 2
  CreateAsset createAsset2;
  createAsset2.domain_id = domain;
  createAsset2.asset_name = asset2name;
  createAsset2.precision = 2;
  txn.commands.push_back(std::make_shared<CreateAsset>(createAsset2));

  // Add 3.00 to user 1
  AddAssetQuantity addAssetQuantity1;
  addAssetQuantity1.asset_id = asset1id;
  addAssetQuantity1.account_id = user1id;
  addAssetQuantity1.amount = iroha::Amount(300, 2);
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity1));

  // Add 2.50 to user 2
  AddAssetQuantity addAssetQuantity2;
  addAssetQuantity2.asset_id = asset2id;
  addAssetQuantity2.account_id = user2id;
  addAssetQuantity2.amount = iroha::Amount(250, 2);
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity2));

  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = iroha::hash(block);
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
    // Check querying account 1
    auto account1 = wsv->getAccount(user1id);
    ASSERT_TRUE(account1);
    ASSERT_EQ(account1->account_id, user1id);
    ASSERT_EQ(account1->domain_id, domain);

    // Check querying account 2
    auto account2 = wsv->getAccount(user2id);
    ASSERT_TRUE(account2);
    ASSERT_EQ(account2->account_id, user2id);
    ASSERT_EQ(account2->domain_id, domain);

    // Check querying account 3
    auto account3 = wsv->getAccount(user3id);
    ASSERT_TRUE(account3);
    ASSERT_EQ(account3->account_id, user3id);
    ASSERT_EQ(account3->domain_id, domain);

    // Check querying asset 1 for user 1
    auto asset1 = wsv->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user1id);
    ASSERT_EQ(asset1->asset_id, asset1id);
    ASSERT_EQ(asset1->balance, iroha::Amount(300, 2));

    // Check querying asset 2 for user 2
    auto asset2 = wsv->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user2id);
    ASSERT_EQ(asset2->asset_id, asset2id);
    ASSERT_EQ(asset2->balance, iroha::Amount(250, 2));
  }

  // 2th tx (user1 -> user2 # asset1)
  txn = Transaction();
  txn.creator_account_id = user1id;

  // Create transfer asset from user 1 to user 2
  TransferAsset transferAsset;
  transferAsset.src_account_id = user1id;
  transferAsset.dest_account_id = user2id;
  transferAsset.asset_id = asset1id;
  transferAsset.amount = iroha::Amount(120, 2);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 2;
  block.prev_hash = block1hash;
  auto block2hash = iroha::hash(block);
  block.hash = block2hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    // Check account asset after transfer assets
    auto asset1 = wsv->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user1id);
    ASSERT_EQ(asset1->asset_id, asset1id);
    ASSERT_EQ(asset1->balance, iroha::Amount(180, 2));

    auto asset2 = wsv->getAccountAsset(user2id, asset1id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user2id);
    ASSERT_EQ(asset2->asset_id, asset1id);
    ASSERT_EQ(asset2->balance, iroha::Amount(120, 2));
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
  transferAsset1.amount = iroha::Amount(150, 2);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset1));

  TransferAsset transferAsset2;
  transferAsset2.src_account_id = user2id;
  transferAsset2.dest_account_id = user1id;
  transferAsset2.asset_id = asset2id;
  transferAsset2.amount = iroha::Amount(10, 2);
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset2));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 3;
  block.prev_hash = block2hash;
  auto block3hash = iroha::hash(block);
  block.hash = block3hash;
  block.txs_number = static_cast<uint16_t>(block.transactions.size());

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto asset1 = wsv->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user2id);
    ASSERT_EQ(asset1->asset_id, asset2id);
    ASSERT_EQ(asset1->balance, iroha::Amount(90, 2));

    auto asset2 = wsv->getAccountAsset(user3id, asset2id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user3id);
    ASSERT_EQ(asset2->asset_id, asset2id);
    ASSERT_EQ(asset2->balance, iroha::Amount(150, 2));

    auto asset3 = wsv->getAccountAsset(user1id, asset2id);
    ASSERT_TRUE(asset3);
    ASSERT_EQ(asset3->account_id, user1id);
    ASSERT_EQ(asset3->asset_id, asset2id);
    ASSERT_EQ(asset3->balance, iroha::Amount(10, 2));
  }

  // Block store tests
  blocks->getBlocks(1, 3).subscribe(
      [block1hash, block2hash, block3hash](auto eachBlock) {
        if (eachBlock.height == 1) {
          EXPECT_EQ(eachBlock.hash, block1hash);
        } else if (eachBlock.height == 2) {
          EXPECT_EQ(eachBlock.hash, block2hash);
        } else if (eachBlock.height == 3) {
          EXPECT_EQ(eachBlock.hash, block3hash);
        }
      });

  blocks->getAccountTransactions(admin).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 9); });
  blocks->getAccountTransactions(user1id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  blocks->getAccountTransactions(user2id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 2); });
  blocks->getAccountTransactions(user3id).subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 0); });

  // (user1 -> user2 # asset1)
  // (user2 -> user3 # asset2)
  // (user2 -> user1 # asset2)
  blocks->getAccountAssetTransactions(user1id, asset1id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 1);
  });
  blocks->getAccountAssetTransactions(user2id, asset1id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 1);
  });
  blocks->getAccountAssetTransactions(user3id, asset1id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 0);
  });
  blocks->getAccountAssetTransactions(user1id, asset2id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 1);
  });
  blocks->getAccountAssetTransactions(user2id, asset2id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 2);
  });
  blocks->getAccountAssetTransactions(user3id, asset2id).subscribe([](auto tx) {
    EXPECT_EQ(tx.commands.size(), 1);
  });
}

TEST_F(AmetsuchiTest, AddSignatoryTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();

  iroha::pubkey_t pubkey1, pubkey2;
  pubkey1.at(0) = 1;
  pubkey2.at(0) = 2;

  auto user1id = "user1@domain";
  auto user2id = "user2@domain";

  // 1st tx (create user1 with pubkey1)
  CreateRole createRole;
  createRole.role_name = "user";
  createRole.permissions = {can_add_peer, can_create_asset, can_get_my_account};

  Transaction txn;
  txn.creator_account_id = "admin1";

  txn.commands.push_back(std::make_shared<CreateRole>(createRole));
  CreateDomain createDomain;
  createDomain.domain_id = "domain";
  createDomain.user_default_role = "user";
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));

  CreateAccount createAccount;
  createAccount.account_name = "user1";
  createAccount.domain_id = "domain";
  createAccount.pubkey = pubkey1;
  createAccount.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = iroha::hash(block);
  block.hash = block1hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &blk, auto &query, const auto &top_hash) {
      return true;
    });
    storage->commit(std::move(ms));
  }

  {
    auto account = wsv->getAccount(user1id);
    ASSERT_TRUE(account);
    ASSERT_EQ(account->account_id, user1id);
    ASSERT_EQ(account->domain_id, createAccount.domain_id);

    auto signatories = wsv->getSignatories(user1id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 1);
    ASSERT_EQ(signatories->at(0), pubkey1);
  }

  // 2nd tx (add sig2 to user1)
  txn = Transaction();
  txn.creator_account_id = user1id;

  auto addSignatory = AddSignatory();
  addSignatory.account_id = user1id;
  addSignatory.pubkey = pubkey2;
  txn.commands.push_back(std::make_shared<AddSignatory>(addSignatory));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 2;
  block.prev_hash = block1hash;
  auto block2hash = iroha::hash(block);
  block.hash = block2hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto account = wsv->getAccount(user1id);
    ASSERT_TRUE(account);

    auto signatories = wsv->getSignatories(user1id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 2);
    ASSERT_EQ(signatories->at(0), pubkey1);
    ASSERT_EQ(signatories->at(1), pubkey2);
  }

  // 3rd tx (create user2 with pubkey1 that is same as user1's key)
  txn = Transaction();
  txn.creator_account_id = "admin2";

  createAccount = CreateAccount();
  createAccount.account_name = "user2";
  createAccount.domain_id = "domain";
  createAccount.pubkey = pubkey1;  // same as user1's pubkey1
  createAccount.json_data = "{}";
  txn.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 3;
  block.prev_hash = block2hash;
  auto block3hash = iroha::hash(block);
  block.hash = block3hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto account1 = wsv->getAccount(user1id);
    ASSERT_TRUE(account1);

    auto account2 = wsv->getAccount(user2id);
    ASSERT_TRUE(account2);

    auto signatories1 = wsv->getSignatories(user1id);
    ASSERT_TRUE(signatories1);
    ASSERT_EQ(signatories1->size(), 2);
    ASSERT_EQ(signatories1->at(0), pubkey1);
    ASSERT_EQ(signatories1->at(1), pubkey2);

    auto signatories2 = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories2);
    ASSERT_EQ(signatories2->size(), 1);
    ASSERT_EQ(signatories2->at(0), pubkey1);
  }

  // 4th tx (remove pubkey1 from user1)
  txn = Transaction();
  txn.creator_account_id = user1id;

  auto removeSignatory = RemoveSignatory();
  removeSignatory.account_id = user1id;
  removeSignatory.pubkey = pubkey1;
  txn.commands.push_back(std::make_shared<RemoveSignatory>(removeSignatory));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 4;
  block.prev_hash = block3hash;
  auto block4hash = iroha::hash(block);
  block.hash = block4hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto account = wsv->getAccount(user1id);
    ASSERT_TRUE(account);

    // user1 has only pubkey2.
    auto signatories1 = wsv->getSignatories(user1id);
    ASSERT_TRUE(signatories1);
    ASSERT_EQ(signatories1->size(), 1);
    ASSERT_EQ(signatories1->at(0), pubkey2);

    // user2 still has pubkey1.
    auto signatories2 = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories2);
    ASSERT_EQ(signatories2->size(), 1);
    ASSERT_EQ(signatories2->at(0), pubkey1);
  }

  // 5th tx (add sig2 to user2 and set quorum = 1)
  txn = Transaction();
  txn.creator_account_id = user2id;

  addSignatory = AddSignatory();
  addSignatory.account_id = user2id;
  addSignatory.pubkey = pubkey2;
  txn.commands.push_back(std::make_shared<AddSignatory>(addSignatory));

  auto seqQuorum = SetQuorum();
  seqQuorum.account_id = user2id;
  seqQuorum.new_quorum = 2;
  txn.commands.push_back(std::make_shared<SetQuorum>(seqQuorum));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 5;
  block.prev_hash = block4hash;
  auto block5hash = iroha::hash(block);
  block.hash = block5hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    auto account = wsv->getAccount(user2id);
    ASSERT_TRUE(account);
    ASSERT_EQ(account->quorum, 2);

    // user2 has pubkey1 and pubkey2.
    auto signatories = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 2);
    ASSERT_EQ(signatories->at(0), pubkey1);
    ASSERT_EQ(signatories->at(1), pubkey2);
  }

  // 6th tx (remove sig2 fro user2: This must success)
  txn = Transaction();
  txn.creator_account_id = user2id;

  removeSignatory = RemoveSignatory();
  removeSignatory.account_id = user2id;
  removeSignatory.pubkey = pubkey2;
  txn.commands.push_back(std::make_shared<RemoveSignatory>(removeSignatory));

  block = Block();
  block.transactions.push_back(txn);
  block.height = 6;
  block.prev_hash = block5hash;
  auto block6hash = iroha::hash(block);
  block.hash = block6hash;
  block.txs_number = block.transactions.size();

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  }

  {
    // user2 only has pubkey1.
    auto signatories = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 1);
    ASSERT_EQ(signatories->at(0), pubkey1);
  }
}

Block getBlock() {
  Transaction txn;
  txn.creator_account_id = "admin1";
  AddPeer add_peer;
  add_peer.address = "192.168.0.0";
  txn.commands.push_back(std::make_shared<AddPeer>(add_peer));
  Block block;
  block.transactions.push_back(txn);
  block.height = 1;
  block.prev_hash.fill(0);
  auto block1hash = iroha::hash(block);
  block.txs_number = block.transactions.size();
  block.hash = block1hash;
  return block;
}

TEST_F(AmetsuchiTest, TestingStorageWhenInsertBlock) {
  auto log = logger::testLog("TestStorage");
  log->info(
      "Test case: create storage "
      "=> insert block "
      "=> assert that inserted");
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
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

TEST_F(AmetsuchiTest, TestingStorageWhenDropAll) {
  auto logger = logger::testLog("TestStorage");
  logger->info(
      "Test case: create storage "
      "=> insert block "
      "=> assert that written"
      " => drop all "
      "=> assert that all deleted ");

  auto log = logger::testLog("TestStorage");
  log->info(
      "Test case: create storage "
      "=> insert block "
      "=> assert that inserted");
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
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
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_EQ(0, wsv->getPeers().value().size());
  new_storage->dropStorage();
}

/**
 * @given initialized storage
 * @when insert block with 2 transactions in
 * @then both of them are found with getTxByHashSync call by hash. Transaction
 * with some other hash is not found.
 */
TEST_F(AmetsuchiTest, FindTxByHashTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto blocks = storage->getBlockQuery();

  iroha::pubkey_t pubkey1, pubkey2;
  pubkey1.at(0) = 1;
  pubkey2.at(0) = 2;

  CreateRole createRole;
  createRole.role_name = "user";
  createRole.permissions = {can_add_peer, can_create_asset, can_get_my_account};

  Transaction tx1;
  tx1.creator_account_id = "admin1";

  tx1.commands.push_back(std::make_shared<CreateRole>(createRole));
  CreateDomain createDomain;
  createDomain.domain_id = "domain";
  createDomain.user_default_role = "user";
  tx1.commands.push_back(std::make_shared<CreateDomain>(createDomain));

  CreateAccount createAccount;
  createAccount.account_name = "user1";
  createAccount.domain_id = "domain";
  createAccount.pubkey = pubkey1;
  tx1.commands.push_back(std::make_shared<CreateAccount>(createAccount));

  CreateRole createRole2;
  createRole2.role_name = "user2";
  createRole2.permissions = {
      can_add_peer, can_create_asset, can_get_my_account};

  Transaction tx2;
  tx2.commands.push_back(std::make_shared<CreateRole>(createRole2));
  CreateDomain createDomain2;
  createDomain2.domain_id = "domain2";
  createDomain2.user_default_role = "user";
  tx2.commands.push_back(std::make_shared<CreateDomain>(createDomain2));

  Block block;
  block.transactions.push_back(tx1);
  block.transactions.push_back(tx2);
  block.height = 1;
  block.prev_hash.fill(0);
  block.txs_number = block.transactions.size();
  block.hash = iroha::hash(block);

  {
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &blk, auto &query, const auto &top_hash) {
      return true;
    });
    storage->commit(std::move(ms));
  }

  // TODO: 31.10.2017 luckychess move tx3hash case into a separate test after
  // ametsuchi_test redesign
  auto tx1hash = iroha::hash(tx1).to_string();
  auto tx2hash = iroha::hash(tx2).to_string();
  auto tx3hash = "some garbage";

  ASSERT_EQ(*blocks->getTxByHashSync(tx1hash), tx1);
  ASSERT_EQ(*blocks->getTxByHashSync(tx2hash), tx2);
  ASSERT_EQ(blocks->getTxByHashSync(tx3hash), boost::none);
}
