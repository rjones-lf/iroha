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
    ASSERT_EQ(account->domain_name, createAccount.domain_id);
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
  iroha::Amount asset_amount(150, 2);
  addAssetQuantity.amount = asset_amount;
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity));
  TransferAsset transferAsset;
  transferAsset.src_account_id = "user1@ru";
  transferAsset.dest_account_id = "user2@ru";
  transferAsset.asset_id = "RUB#ru";
  transferAsset.description = "test transfer";
  iroha::Amount transfer_amount(100, 2);
  transferAsset.amount = transfer_amount;
  txn.commands.push_back(std::make_shared<TransferAsset>(transferAsset));

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

  blocks->getAccountTransactions("admin1").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 2); });
  blocks->getAccountTransactions("admin2").subscribe(
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 4); });

  blocks->getAccountAssetTransactions("user1@ru", "RUB#ru")
      .subscribe([](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
  blocks->getAccountAssetTransactions("user2@ru", "RUB#ru")
      .subscribe([](auto tx) { EXPECT_EQ(tx.commands.size(), 1); });
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
  addAssetQuantity1.amount = iroha::Amount(300, 2);
  txn.commands.push_back(std::make_shared<AddAssetQuantity>(addAssetQuantity1));
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
    auto account1 = wsv->getAccount(user1id);
    ASSERT_TRUE(account1);
    ASSERT_EQ(account1->account_id, user1id);
    ASSERT_EQ(account1->domain_name, domain);
    auto account2 = wsv->getAccount(user2id);
    ASSERT_TRUE(account2);
    ASSERT_EQ(account2->account_id, user2id);
    ASSERT_EQ(account2->domain_name, domain);
    auto account3 = wsv->getAccount(user3id);
    ASSERT_TRUE(account3);
    ASSERT_EQ(account3->account_id, user3id);
    ASSERT_EQ(account3->domain_name, domain);

    auto asset1 = wsv->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(asset1);
    ASSERT_EQ(asset1->account_id, user1id);
    ASSERT_EQ(asset1->asset_id, asset1id);
    ASSERT_EQ(asset1->balance, iroha::Amount(300, 2));
    auto asset2 = wsv->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(asset2);
    ASSERT_EQ(asset2->account_id, user2id);
    ASSERT_EQ(asset2->asset_id, asset2id);
    ASSERT_EQ(asset2->balance, iroha::Amount(250, 2));
  }

  // 2th tx (user1 -> user2 # asset1)
  txn = Transaction();
  txn.creator_account_id = user1id;
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
      [](auto tx) { EXPECT_EQ(tx.commands.size(), 8); });
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
  Transaction txn;
  txn.creator_account_id = "admin1";
  CreateDomain createDomain;
  createDomain.domain_name = "domain";
  txn.commands.push_back(std::make_shared<CreateDomain>(createDomain));
  CreateAccount createAccount;
  createAccount.account_name = "user1";
  createAccount.domain_id = "domain";
  createAccount.pubkey = pubkey1;
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
    ASSERT_EQ(account->domain_name, createAccount.domain_id);

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

TEST_F(AmetsuchiTest, GetAccountTransactionsWithPagerTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();

  const auto adminid = std::string("admin@maindomain");
  const auto domain1name = std::string("domain1");
  const auto user1name = std::string("alice");
  const auto user2name = std::string("bob");
  const auto user1id = user1name + "@" + domain1name;
  const auto user2id = user2name + "@" + domain1name;
  const auto alice_assetname = std::string("alice_asset");
  const auto alice_domain = std::string("alice_domain");
  const auto alice_assetid = alice_assetname + "#" + alice_domain;
  const auto bob_assetname = std::string("bob_asset");
  const auto bob_domain = std::string("bob_domain");
  const auto bob_assetid = bob_assetname + "#" + bob_domain;

  auto commit_block = [&storage](auto block, auto height, auto prev_hash) {
    block.height = height;
    block.created_ts = 0;
    block.txs_number = static_cast<uint16_t>(block.transactions.size());
    block.prev_hash = prev_hash;
    block.hash = iroha::hash(block);
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  };

  Transaction tx1, tx2;

  Block block1;
  {
    // tx1: Admin send CreateDomain(domain1, domain2), CreateAccount(alice, bob)
    tx1 = Transaction{};
    tx1.creator_account_id = adminid;
    tx1.commands = {std::make_shared<CreateDomain>(domain1name),
                    std::make_shared<CreateAccount>(
                        user1name, domain1name, iroha::pubkey_t{}),
                    std::make_shared<CreateAccount>(
                        user2name, domain1name, iroha::pubkey_t{})};
    block1.transactions.push_back(tx1);

    // tx2: Alice send CreateDomain
    tx2 = Transaction{};
    tx2.creator_account_id = user1id;
    tx2.commands = {std::make_shared<CreateDomain>(alice_domain)};
    block1.transactions.push_back(tx2);
  }

  commit_block(block1, 1, iroha::hash256_t{});

  blocks->getTopBlocks(1).subscribe([](auto block) {
    EXPECT_EQ(2, block.transactions.size());
    EXPECT_EQ(3, block.transactions[0].commands.size());
    EXPECT_EQ(1, block.transactions[1].commands.size());
  });

  auto account1 = wsv->getAccount(user1id);
  ASSERT_TRUE(account1);
  ASSERT_STREQ(user1id.c_str(), account1->account_id.c_str());

  auto account2 = wsv->getAccount(user2id);
  ASSERT_TRUE(account2);
  ASSERT_STREQ(user2id.c_str(), account2->account_id.c_str());

  Block block2;
  {
    // tx1: Alice send CreateAsset
    tx1 = Transaction{};
    tx1.creator_account_id = user1id;
    tx1.commands = {
        std::make_shared<CreateAsset>(alice_assetname, alice_domain, 0)};
    block2.transactions.push_back(tx1);

    // tx2: Bob send CreateDomain, CreateAsset
    tx2 = Transaction{};
    tx2.creator_account_id = user2id;
    tx2.commands = {
        std::make_shared<CreateDomain>(bob_domain),
        std::make_shared<CreateAsset>(bob_assetname, bob_domain, 0)};
    block2.transactions.push_back(tx2);
  }

  commit_block(block2, 2, block1.hash);

  blocks->getTopBlocks(1).subscribe([&](auto block) {
    EXPECT_EQ(2, block.transactions.size());
    EXPECT_EQ(1, block.transactions[0].commands.size());
    EXPECT_EQ(2, block.transactions[1].commands.size());
  });

  ASSERT_TRUE(wsv->getAsset(alice_assetid));
  ASSERT_TRUE(wsv->getAsset(bob_assetid));

  // When query with limit 0
  blocks->getAccountTransactionsWithPager(user1id, Pager{iroha::hash256_t{}, 0})
      .subscribe([](auto) {
        FAIL() << "Pager with limit 0 cannot subscribe any transactions";
      });

  // When query with a last alice's tx.
  std::vector<Transaction> results;
  blocks->getAccountTransactionsWithPager(user1id, Pager{iroha::hash256_t{}, 1})
      .subscribe([&](auto tx) { results.push_back(tx); });

  ASSERT_EQ(1, results.size());
  {
    EXPECT_EQ(1, results[0].commands.size());
    auto c = std::dynamic_pointer_cast<CreateAsset>(results[0].commands[0]);
    ASSERT_TRUE(c);
    EXPECT_EQ(alice_assetname, c->asset_name);
    EXPECT_EQ(alice_domain, c->domain_id);
  }

  // When query with last two alice's txs.
  results.clear();
  blocks->getAccountTransactionsWithPager(user1id, Pager{iroha::hash256_t{}, 2})
      .subscribe([&](auto tx) { results.push_back(tx); });

  ASSERT_EQ(2, results.size());
  {
    ASSERT_EQ(1, results[0].commands.size());
    auto c1 = std::dynamic_pointer_cast<CreateAsset>(results[0].commands[0]);
    ASSERT_TRUE(c1);
    EXPECT_EQ(alice_assetname, c1->asset_name);
    EXPECT_EQ(alice_domain, c1->domain_id);
  }
  {
    ASSERT_EQ(1, results[1].commands.size());
    auto c1 = std::dynamic_pointer_cast<CreateDomain>(results[1].commands[0]);
    ASSERT_TRUE(c1);
    EXPECT_EQ(alice_domain, c1->domain_name);
  }

  // When query with alice's txs with overflowed limit.
  results.clear();
  blocks
      ->getAccountTransactionsWithPager(user1id, Pager{iroha::hash256_t{}, 100})
      .subscribe([&](auto tx) { results.push_back(tx); });

  ASSERT_EQ(2, results.size());
  {
    ASSERT_EQ(1, results[0].commands.size());
    auto c1 = std::dynamic_pointer_cast<CreateAsset>(results[0].commands[0]);
    ASSERT_TRUE(c1);
    EXPECT_EQ(alice_assetname, c1->asset_name);
    EXPECT_EQ(alice_domain, c1->domain_id);
  }
  {
    ASSERT_EQ(1, results[1].commands.size());
    auto c1 = std::dynamic_pointer_cast<CreateDomain>(results[1].commands[0]);
    ASSERT_TRUE(c1);
    EXPECT_EQ(alice_domain, c1->domain_name);
  }

  // When query bob's txs with overflowed limit.
  results.clear();
  blocks
      ->getAccountTransactionsWithPager(
          user2id, Pager{iroha::hash256_t{}, 100})
      .subscribe([&](auto tx) { results.push_back(tx); });

  ASSERT_EQ(1, results.size());
  {
    ASSERT_EQ(2, results[0].commands.size());
    auto c1 = std::dynamic_pointer_cast<CreateDomain>(results[0].commands[0]);
    ASSERT_TRUE(c1);
    EXPECT_EQ(bob_domain, c1->domain_name);
    auto c2 = std::dynamic_pointer_cast<CreateAsset>(results[0].commands[1]);
    ASSERT_TRUE(c2);
    EXPECT_EQ(bob_domain, c2->domain_id);
    EXPECT_EQ(bob_assetname, c2->asset_name);
  }
}

TEST_F(AmetsuchiTest, GetAccountAssetsTransactionsWithPagerTest) {
  auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();

  const std::string adminid = "admin@maindomain";
  const std::string domain1name = "domain1";
  const std::string domain2name = "domain2";
  const std::string user1name = "alice";
  const std::string user2name = "bob";
  const std::string user1id = "alice@domain1";
  const std::string user2id = "bob@domain1";

  auto commit_block = [&storage](auto block, auto height, auto prev_hash) {
    block.height = height;
    block.created_ts = 0;
    block.txs_number = static_cast<uint16_t>(block.transactions.size());
    block.prev_hash = prev_hash;
    block.hash = iroha::hash(block);
    auto ms = storage->createMutableStorage();
    ms->apply(block, [](const auto &, auto &, const auto &) { return true; });
    storage->commit(std::move(ms));
  };

  Transaction tx1, tx2;

  Block block1;
  {
    tx1 = Transaction{};
    tx1.creator_account_id = adminid;
    tx1.commands = {std::make_shared<CreateDomain>(domain1name),
                    std::make_shared<CreateDomain>(domain2name),
                    std::make_shared<CreateAccount>(
                        user1name, domain1name, iroha::pubkey_t{}),
                    std::make_shared<CreateAccount>(
                        user2name, domain1name, iroha::pubkey_t{})};
    block1.transactions.push_back(tx1);
  }

  commit_block(block1, 1, iroha::hash256_t{});

  blocks->getTopBlocks(1).subscribe([](auto block) {
    EXPECT_EQ(1, block.transactions.size());
    EXPECT_EQ(4, block.transactions[0].commands.size());
  });

  // Then Account alice@domain1 should be load.
  auto account1 = wsv->getAccount(user1id);
  ASSERT_TRUE(account1);
  ASSERT_STREQ(user1id.c_str(), account1->account_id.c_str());

  // Then Account bob@domain1 should be load.
  auto account2 = wsv->getAccount(user2id);
  ASSERT_TRUE(account2);
  ASSERT_STREQ(user2id.c_str(), account2->account_id.c_str());

  const auto asset1name = std::string("irh");
  const auto asset1id = asset1name + "#" + domain1name;
  const auto asset1prec = 1;
  const auto asset2name = std::string("moeka");
  const auto asset2id = asset2name + "#" + domain2name;
  const auto asset2prec = 2;

  Block block2;
  {
    // Admin applies CreateAsset irh@domain1, CreateAsset moeka@domain2
    tx1 = Transaction{};
    tx1.creator_account_id = adminid;
    tx1.commands = {
        std::make_shared<CreateAsset>(asset1name, domain1name, asset1prec),
        std::make_shared<CreateAsset>(asset2name, domain2name, asset2prec)};
    block2.transactions.push_back(tx1);
  }

  commit_block(block2, 2, block1.hash);

  blocks->getTopBlocks(1).subscribe([&](auto block) {
    EXPECT_EQ(1, block.transactions.size());
    EXPECT_EQ(2, block.transactions[0].commands.size());
  });

  ASSERT_TRUE(wsv->getAsset(asset1id));
  ASSERT_TRUE(wsv->getAsset(asset2id));
  ASSERT_TRUE(wsv->getAccount(user1id));

  Block block3;
  {
    // Admin applies AddAssetQuantity with Alice's irh@domain1 and moeka@domain2
    // wallet.
    tx1 = Transaction{};
    tx1.creator_account_id = adminid;
    tx1.commands = {std::make_shared<AddAssetQuantity>(
                        user1id, asset1id, iroha::Amount(1234, asset1prec)),
                    std::make_shared<AddAssetQuantity>(
                        user2id, asset1id, iroha::Amount(100, asset1prec)),
                    std::make_shared<AddAssetQuantity>(
                        user2id, asset2id, iroha::Amount(200, asset2prec))};
    block3.transactions.push_back(tx1);
  }

  commit_block(block3, 3, block2.hash);
  blocks->getTopBlocks(1).subscribe([&](auto block) {
    EXPECT_EQ(1, block.transactions.size());
    EXPECT_EQ(3, block.transactions[0].commands.size());
  });

  {
    auto acct_asset = wsv->getAccountAsset(user1id, asset1id);
    ASSERT_TRUE(acct_asset);
    ASSERT_EQ(iroha::Amount(1234, asset1prec), acct_asset->balance);
  }
  {
    auto acct_asset = wsv->getAccountAsset(user2id, asset1id);
    ASSERT_TRUE(acct_asset);
    ASSERT_EQ(iroha::Amount(100, asset1prec), acct_asset->balance);
  }
  {
    auto acct_asset = wsv->getAccountAsset(user2id, asset2id);
    ASSERT_TRUE(acct_asset);
    ASSERT_EQ(iroha::Amount(200, asset2prec), acct_asset->balance);
  }

  Block block4;
  {
    tx1 = Transaction{};
    tx1.creator_account_id = adminid;
    tx1.commands = {
        std::make_shared<CreateDomain>("dummy"),
        std::make_shared<TransferAsset>(
            user1id, user2id, asset1id, iroha::Amount(1234, asset1prec)),
        std::make_shared<CreateAccount>(
            "dummy_acct_1", "dummy", iroha::pubkey_t{})};
    block4.transactions.push_back(tx1);

    tx2 = Transaction{};
    tx2.creator_account_id = adminid;
    tx2.commands = {std::make_shared<CreateAccount>(
                        "dummy_acct_2", "dummy", iroha::pubkey_t{}),
                    std::make_shared<AddAssetQuantity>(
                        user1id, asset2id, iroha::Amount(500, asset2prec))};
    block4.transactions.push_back(tx2);
  };

  commit_block(block4, 4, block3.hash);

  blocks->getTopBlocks(1).subscribe([&](auto block) {
    EXPECT_EQ(2, block.transactions.size());
    EXPECT_EQ(3, block.transactions[0].commands.size());
    EXPECT_EQ(2, block.transactions[1].commands.size());
  });

  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id}, Pager{iroha::hash256_t{}, 0})
      .subscribe(
          [](auto) { FAIL() << "subscribe shouldn't occur with limit 0"; });

  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id}, Pager{iroha::hash256_t{}, 1})
      .subscribe([&](auto tx) {
        EXPECT_EQ(3, tx.commands.size());
        EXPECT_TRUE(std::dynamic_pointer_cast<CreateDomain>(tx.commands[0]));
        EXPECT_TRUE(std::dynamic_pointer_cast<TransferAsset>(tx.commands[1]));
        EXPECT_TRUE(std::dynamic_pointer_cast<CreateAccount>(tx.commands[2]));
      });

  std::vector<Transaction> result;
  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id}, Pager{iroha::hash256_t{}, 2})
      .subscribe([&](auto tx) { result.push_back(tx); });

  ASSERT_EQ(2, result.size());
  ASSERT_EQ(3, result[0].commands.size());
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateDomain>(result[0].commands[0]));
  EXPECT_TRUE(std::dynamic_pointer_cast<TransferAsset>(result[0].commands[1]));
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateAccount>(result[0].commands[2]));
  ASSERT_EQ(3, result[1].commands.size());
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[0]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[1]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[2]));

  result.clear();
  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id}, Pager{iroha::hash256_t{}, 100})
      .subscribe([&](auto tx) { result.push_back(tx); });

  ASSERT_EQ(2, result.size());
  ASSERT_EQ(3, result[0].commands.size());
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateDomain>(result[0].commands[0]));
  EXPECT_TRUE(std::dynamic_pointer_cast<TransferAsset>(result[0].commands[1]));
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateAccount>(result[0].commands[2]));
  ASSERT_EQ(3, result[1].commands.size());
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[0]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[1]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[1].commands[2]));

  result.clear();
  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id, asset2id}, Pager{iroha::hash256_t{}, 100})
      .subscribe([&](auto tx) { result.push_back(tx); });

  ASSERT_EQ(3, result.size());
  ASSERT_EQ(2, result[0].commands.size());
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateAccount>(result[0].commands[0]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[0].commands[1]));
  ASSERT_EQ(3, result[1].commands.size());
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateDomain>(result[1].commands[0]));
  EXPECT_TRUE(std::dynamic_pointer_cast<TransferAsset>(result[1].commands[1]));
  EXPECT_TRUE(std::dynamic_pointer_cast<CreateAccount>(result[1].commands[2]));
  ASSERT_EQ(3, result[2].commands.size());
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[2].commands[0]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[2].commands[1]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[2].commands[2]));

  auto w = wsv->getAccountAsset(user1id, asset2id);
  ASSERT_TRUE(w);
  ASSERT_EQ(iroha::Amount(500, asset2prec), w->balance);

  // Get transactions until a tx which has (CreateDomain, TransferAsset,
  // CreateAccount). Tx which has until_tx_hash is excluded.
  auto until_tx_hash = iroha::hash(result[1]);

  result.clear();
  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {asset1id, asset2id}, Pager{until_tx_hash, 100})
      .subscribe([&](auto tx) { result.push_back(tx); });

  ASSERT_EQ(1, result.size());
  ASSERT_EQ(3, result[0].commands.size());
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[0].commands[0]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[0].commands[1]));
  EXPECT_TRUE(
      std::dynamic_pointer_cast<AddAssetQuantity>(result[0].commands[2]));

  blocks
      ->getAccountAssetsTransactionsWithPager(
          user1id, {}, Pager{iroha::hash256_t{}, 100})
      .subscribe(
          [&](auto tx) { FAIL() << "Shouldn't subscribe if no assets."; });
}
