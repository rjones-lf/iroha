/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "ametsuchi/impl/postgres_block_query.hpp"
#include "ametsuchi/impl/postgres_ordering_service_persistent_state.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/wsv_restorer_impl.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "ametsuchi/temporary_wsv.hpp"
#include "builders/default_builders.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/result_fixture.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace iroha::ametsuchi;
using namespace framework::test_subscriber;
using namespace shared_model::interface::permissions;

auto zero_string = std::string(32, '0');
auto fake_hash = shared_model::crypto::Hash(zero_string);
auto fake_pubkey = shared_model::crypto::PublicKey(zero_string);

// Allows to print amount string in case of test failure
namespace shared_model {
  namespace interface {
    void PrintTo(const Amount &amount, std::ostream *os) {
      *os << amount.toString();
    }
  }  // namespace interface
}  // namespace shared_model

/**
 * Validate getAccountTransaction with given parameters
 * @tparam B block query type
 * @param blocks block query object
 * @param account id to query
 * @param call_count number of observable calls
 * @param command_count number of commands in transaction
 */
template <typename B>
void validateAccountTransactions(B &&blocks,
                                 const std::string &account,
                                 int call_count,
                                 int command_count) {
  auto txs = blocks->getAccountTransactions(account);
  ASSERT_EQ(txs.size(), call_count);
  std::for_each(txs.begin(), txs.end(), [&](const auto &tx) {
    EXPECT_EQ(tx->commands().size(), command_count);
  });
}

/**
 * Validate getAccountAssetTransactions with given parameters
 * @tparam B block query type
 * @param blocks block query object
 * @param account id to query
 * @param asset id to query
 * @param call_count number of observable calls
 * @param command_count number of commands in transaction
 */
template <typename B>
void validateAccountAssetTransactions(B &&blocks,
                                      const std::string &account,
                                      const std::string &asset,
                                      int call_count,
                                      int command_count) {
  auto txs = blocks->getAccountAssetTransactions(account, asset);
  ASSERT_EQ(txs.size(), call_count);
  std::for_each(txs.begin(), txs.end(), [&](const auto &tx) {
    EXPECT_EQ(tx->commands().size(), command_count);
  });
}

/**
 * Validate getAccountAsset with given parameters
 * @tparam W WSV query type
 * @param wsv WSV query object
 * @param account id to query
 * @param asset id to query
 * @param amount to validate
 */
template <typename W>
void validateAccountAsset(W &&wsv,
                          const std::string &account,
                          const std::string &asset,
                          const shared_model::interface::Amount &amount) {
  auto account_asset = wsv->getAccountAsset(account, asset);
  ASSERT_TRUE(account_asset);
  ASSERT_EQ((*account_asset)->accountId(), account);
  ASSERT_EQ((*account_asset)->assetId(), asset);
  ASSERT_EQ((*account_asset)->balance(), amount);
}

/**
 * Validate getAccount with given parameters
 * @tparam W WSV query type
 * @param wsv WSV query object
 * @param id account to query
 * @param domain id to validate
 */
template <typename W>
void validateAccount(W &&wsv,
                     const std::string &id,
                     const std::string &domain) {
  auto account = wsv->getAccount(id);
  ASSERT_TRUE(account);
  ASSERT_EQ((*account)->accountId(), id);
  ASSERT_EQ((*account)->domainId(), domain);
}

/**
 * Apply block to given storage
 * @tparam S storage type
 * @param storage storage object
 * @param block to apply
 */
template <typename S>
void apply(S &&storage, const shared_model::interface::Block &block) {
  std::unique_ptr<MutableStorage> ms;
  auto storageResult = storage->createMutableStorage();
  storageResult.match(
      [&](iroha::expected::Value<std::unique_ptr<MutableStorage>> &_storage) {
        ms = std::move(_storage.value);
      },
      [](iroha::expected::Error<std::string> &error) {
        FAIL() << "MutableStorage: " << error.error;
      });
  ms->apply(block);
  storage->commit(std::move(ms));
}

TEST_F(AmetsuchiTest, GetBlocksCompletedWhenCalled) {
  // Commit block => get block => observable completed
  ASSERT_TRUE(storage);
  auto blocks = storage->getBlockQuery();

  auto block = TestBlockBuilder().height(1).prevHash(fake_hash).build();

  apply(storage, block);

  ASSERT_EQ(*blocks->getBlocks(1, 1)[0], block);
}

TEST_F(AmetsuchiTest, SampleTest) {
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();

  const auto domain = "ru", user1name = "userone", user2name = "usertwo",
             user1id = "userone@ru", user2id = "usertwo@ru", assetname = "rub",
             assetid = "rub#ru";

  // Block 1
  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId("admin1")
          .createRole("user",
                      {Role::kAddPeer, Role::kCreateAsset, Role::kGetMyAccount})
          .createDomain(domain, "user")
          .createAccount(user1name, domain, fake_pubkey)
          .build());
  auto block1 = TestBlockBuilder()
                    .transactions(txs)
                    .height(1)
                    .prevHash(fake_hash)
                    .build();

  apply(storage, block1);

  validateAccount(wsv, user1id, domain);

  // Block 2
  txs.clear();
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId(user1id)
          .createAccount(user2name, domain, fake_pubkey)
          .createAsset(assetname, domain, 1)
          .addAssetQuantity(assetid, "150.0")
          .transferAsset(user1id, user2id, assetid, "Transfer asset", "100.0")
          .build());
  auto block2 = TestBlockBuilder()
                    .transactions(txs)
                    .height(2)
                    .prevHash(block1.hash())
                    .build();

  apply(storage, block2);
  validateAccountAsset(
      wsv, user1id, assetid, shared_model::interface::Amount("50.0"));
  validateAccountAsset(
      wsv, user2id, assetid, shared_model::interface::Amount("100.0"));

  // Block store tests
  auto hashes = {block1.hash(), block2.hash()};

  auto stored_blocks = blocks->getBlocks(1, 2);
  ASSERT_EQ(2, stored_blocks.size());
  for (size_t i = 0; i < stored_blocks.size(); i++) {
    EXPECT_EQ(*(hashes.begin() + i), stored_blocks[i]->hash());
  }

  validateAccountTransactions(blocks, "admin1", 1, 3);
  validateAccountTransactions(blocks, user1id, 1, 4);
  validateAccountTransactions(blocks, "non_existing_user", 0, 0);

  validateAccountAssetTransactions(blocks, user1id, assetid, 1, 4);
  validateAccountAssetTransactions(blocks, user2id, assetid, 1, 4);
  validateAccountAssetTransactions(
      blocks, "non_existing_user", "non_existing_asset", 0, 0);
}

TEST_F(AmetsuchiTest, PeerTest) {
  auto wsv = storage->getWsvQuery();

  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(TestTransactionBuilder()
                    .addPeer("192.168.9.1:50051", fake_pubkey)
                    .build());

  auto block = TestBlockBuilder().transactions(txs).prevHash(fake_hash).build();

  apply(storage, block);

  auto peers = wsv->getPeers();
  ASSERT_TRUE(peers);
  ASSERT_EQ(peers->size(), 1);
  ASSERT_EQ(peers->at(0)->address(), "192.168.9.1:50051");

  ASSERT_EQ(peers->at(0)->pubkey(), fake_pubkey);
}

TEST_F(AmetsuchiTest, queryGetAccountAssetTransactionsTest) {
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();
  auto blocks = storage->getBlockQuery();

  const auto admin = "admin", domain = "domain", user1name = "userone",
             user2name = "usertwo", user3name = "userthree",
             user1id = "userone@domain", user2id = "usertwo@domain",
             user3id = "userthree@domain", asset1name = "assetone",
             asset2name = "assettwo", asset1id = "assetone#domain",
             asset2id = "assettwo#domain";

  // 1st tx
  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId(admin)
          .createRole("user",
                      {Role::kAddPeer, Role::kCreateAsset, Role::kGetMyAccount})
          .createDomain(domain, "user")
          .createAccount(user1name, domain, fake_pubkey)
          .createAccount(user2name, domain, fake_pubkey)
          .createAccount(user3name, domain, fake_pubkey)
          .createAsset(asset1name, domain, 1)
          .createAsset(asset2name, domain, 1)
          .build());
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user1id)
                    .addAssetQuantity(asset1id, "300.0")
                    .build());
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user2id)
                    .addAssetQuantity(asset2id, "250.0")
                    .build());

  auto block1 = TestBlockBuilder()
                    .height(1)
                    .transactions(txs)
                    .prevHash(fake_hash)
                    .build();

  apply(storage, block1);

  // Check querying accounts
  for (const auto &id : {user1id, user2id, user3id}) {
    validateAccount(wsv, id, domain);
  }

  // Check querying assets for users
  validateAccountAsset(
      wsv, user1id, asset1id, shared_model::interface::Amount("300.0"));
  validateAccountAsset(
      wsv, user2id, asset2id, shared_model::interface::Amount("250.0"));

  // 2th tx (user1 -> user2 # asset1)
  txs.clear();
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId(user1id)
          .transferAsset(user1id, user2id, asset1id, "Transfer asset", "120.0")
          .build());
  auto block2 = TestBlockBuilder()
                    .transactions(txs)
                    .height(2)
                    .prevHash(block1.hash())
                    .build();

  apply(storage, block2);

  // Check account asset after transfer assets
  validateAccountAsset(
      wsv, user1id, asset1id, shared_model::interface::Amount("180.0"));
  validateAccountAsset(
      wsv, user2id, asset1id, shared_model::interface::Amount("120.0"));

  // 3rd tx
  //   (user2 -> user3 # asset2)
  //   (user2 -> user1 # asset2)
  txs.clear();
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId(user2id)
          .transferAsset(user2id, user3id, asset2id, "Transfer asset", "150.0")
          .transferAsset(user2id, user1id, asset2id, "Transfer asset", "10.0")
          .build());

  auto block3 = TestBlockBuilder()
                    .transactions(txs)
                    .height(3)
                    .prevHash(block2.hash())
                    .build();

  apply(storage, block3);

  validateAccountAsset(
      wsv, user2id, asset2id, shared_model::interface::Amount("90.0"));
  validateAccountAsset(
      wsv, user3id, asset2id, shared_model::interface::Amount("150.0"));
  validateAccountAsset(
      wsv, user1id, asset2id, shared_model::interface::Amount("10.0"));

  // Block store test
  auto hashes = {block1.hash(), block2.hash(), block3.hash()};

  auto stored_blocks = blocks->getBlocks(1, 3);
  ASSERT_EQ(3, stored_blocks.size());
  for (size_t i = 0; i < stored_blocks.size(); i++) {
    EXPECT_EQ(*(hashes.begin() + i), stored_blocks[i]->hash());
  }

  validateAccountTransactions(blocks, admin, 1, 7);
  validateAccountTransactions(blocks, user3id, 0, 0);

  // (user1 -> user2 # asset1)
  // (user2 -> user3 # asset2)
  // (user2 -> user1 # asset2)
  validateAccountAssetTransactions(blocks, user1id, asset1id, 1, 1);
  validateAccountAssetTransactions(blocks, user2id, asset1id, 1, 1);
  validateAccountAssetTransactions(blocks, user3id, asset1id, 0, 0);
  validateAccountAssetTransactions(blocks, user1id, asset2id, 1, 2);
  validateAccountAssetTransactions(blocks, user2id, asset2id, 1, 2);
  validateAccountAssetTransactions(blocks, user3id, asset2id, 1, 2);
}

TEST_F(AmetsuchiTest, AddSignatoryTest) {
  ASSERT_TRUE(storage);
  auto wsv = storage->getWsvQuery();

  shared_model::crypto::PublicKey pubkey1(std::string(32, '1'));
  shared_model::crypto::PublicKey pubkey2(std::string(32, '2'));

  auto user1id = "userone@domain";
  auto user2id = "usertwo@domain";

  // 1st tx (create user1 with pubkey1)
  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId("adminone")
          .createRole("user",
                      {Role::kAddPeer, Role::kCreateAsset, Role::kGetMyAccount})
          .createDomain("domain", "user")
          .createAccount("userone", "domain", pubkey1)
          .build());
  auto block1 = TestBlockBuilder()
                    .transactions(txs)
                    .height(1)
                    .prevHash(fake_hash)
                    .build();

  apply(storage, block1);

  {
    auto account_opt = wsv->getAccount(user1id);
    ASSERT_TRUE(account_opt);
    auto account = account_opt.value();
    ASSERT_EQ(account->accountId(), user1id);
    ASSERT_EQ(account->domainId(), "domain");

    auto signatories = wsv->getSignatories(user1id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 1);
    ASSERT_EQ(signatories->at(0), pubkey1);
  }

  // 2nd tx (add sig2 to user1)
  txs.clear();
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user1id)
                    .addSignatory(user1id, pubkey2)
                    .build());

  auto block2 = TestBlockBuilder()
                    .transactions(txs)
                    .height(2)
                    .prevHash(block1.hash())
                    .build();

  apply(storage, block2);

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
  txs.clear();
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId("admintwo")
                    .createAccount("usertwo", "domain", pubkey1)
                    .build());

  auto block3 = TestBlockBuilder()
                    .transactions(txs)
                    .height(3)
                    .prevHash(block2.hash())
                    .build();

  apply(storage, block3);

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
  txs.clear();
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user1id)
                    .removeSignatory(user1id, pubkey1)
                    .build());

  auto block4 = TestBlockBuilder()
                    .transactions(txs)
                    .height(4)
                    .prevHash(block3.hash())
                    .build();

  apply(storage, block4);

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
  txs.clear();
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user1id)
                    .addSignatory(user2id, pubkey2)
                    .setAccountQuorum(user2id, 2)
                    .build());

  auto block5 = TestBlockBuilder()
                    .transactions(txs)
                    .height(5)
                    .prevHash(block4.hash())
                    .build();

  apply(storage, block5);

  {
    auto account_opt = wsv->getAccount(user2id);
    ASSERT_TRUE(account_opt);
    auto &account = account_opt.value();
    ASSERT_EQ(account->quorum(), 2);

    // user2 has pubkey1 and pubkey2.
    auto signatories = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 2);
    ASSERT_EQ(signatories->at(0), pubkey1);
    ASSERT_EQ(signatories->at(1), pubkey2);
  }

  // 6th tx (remove sig2 fro user2: This must success)
  txs.clear();
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId(user2id)
                    .removeSignatory(user2id, pubkey2)
                    .setAccountQuorum(user2id, 2)
                    .build());

  auto block6 = TestBlockBuilder()
                    .transactions(txs)
                    .height(6)
                    .prevHash(block5.hash())
                    .build();

  apply(storage, block6);

  {
    // user2 only has pubkey1.
    auto signatories = wsv->getSignatories(user2id);
    ASSERT_TRUE(signatories);
    ASSERT_EQ(signatories->size(), 1);
    ASSERT_EQ(signatories->at(0), pubkey1);
  }
}

shared_model::proto::Block getBlock() {
  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(TestTransactionBuilder()
                    .creatorAccountId("adminone")
                    .addPeer("192.168.0.0:10001", fake_pubkey)
                    .build());

  auto block = TestBlockBuilder()
                   .transactions(txs)
                   .height(1)
                   .prevHash(fake_hash)
                   .build();
  return block;
}

TEST_F(AmetsuchiTest, TestingStorageWhenInsertBlock) {
  auto log = logger::testLog("TestStorage");
  log->info(
      "Test case: create storage "
      "=> insert block "
      "=> assert that inserted");
  ASSERT_TRUE(storage);
  auto wrapper = make_test_subscriber<CallExact>(storage->on_commit(), 1);
  wrapper.subscribe();
  auto wsv = storage->getWsvQuery();
  ASSERT_EQ(0, wsv->getPeers().value().size());

  log->info("Try insert block");

  auto inserted = storage->insertBlock(getBlock());
  ASSERT_TRUE(inserted);

  log->info("Request ledger information");

  ASSERT_NE(0, wsv->getPeers().value().size());

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given created storage
 * @when commit block
 * @then committed block is emitted to observable
 */
TEST_F(AmetsuchiTest, TestingStorageWhenCommitBlock) {
  ASSERT_TRUE(storage);

  auto expected_block = getBlock();

  // create test subscriber to check if committed block is as expected
  auto wrapper = make_test_subscriber<CallExact>(storage->on_commit(), 1);
  wrapper.subscribe([&expected_block](const auto &block) {
    ASSERT_EQ(*block, expected_block);
  });

  std::unique_ptr<MutableStorage> mutable_storage;
  storage->createMutableStorage().match(
      [&mutable_storage](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                             &mut_storage) {
        mutable_storage = std::move(mut_storage.value);
      },
      [](const auto &) { FAIL() << "Mutable storage cannot be created"; });

  mutable_storage->apply(expected_block);

  storage->commit(std::move(mutable_storage));

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized storage
 * @when insert block with 2 transactions in
 * @then both of them are found with getTxByHashSync call by hash. Transaction
 * with some other hash is not found.
 */
TEST_F(AmetsuchiTest, FindTxByHashTest) {
  ASSERT_TRUE(storage);
  auto blocks = storage->getBlockQuery();

  shared_model::crypto::PublicKey pubkey1(std::string(32, '1'));
  shared_model::crypto::PublicKey pubkey2(std::string(32, '2'));

  std::vector<shared_model::proto::Transaction> txs;
  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId("admin1")
          .createRole("user",
                      {Role::kAddPeer, Role::kCreateAsset, Role::kGetMyAccount})
          .createDomain("domain", "user")
          .createAccount("userone", "domain", pubkey1)
          .build());

  txs.push_back(
      TestTransactionBuilder()
          .creatorAccountId("admin1")
          .createRole("usertwo",
                      {Role::kAddPeer, Role::kCreateAsset, Role::kGetMyAccount})
          .createDomain("domaintwo", "user")
          .build());

  auto block = TestBlockBuilder()
                   .transactions(txs)
                   .height(1)
                   .prevHash(fake_hash)
                   .build();

  apply(storage, block);

  // TODO: 31.10.2017 luckychess move tx3hash case into a separate test after
  // ametsuchi_test redesign
  auto tx1hash = txs.at(0).hash();
  auto tx2hash = txs.at(1).hash();
  auto tx3hash = shared_model::crypto::Hash("some garbage");

  auto tx1 = blocks->getTxByHashSync(tx1hash);
  ASSERT_TRUE(tx1);

  auto tx2 = blocks->getTxByHashSync(tx2hash);
  ASSERT_TRUE(tx2);

  ASSERT_EQ(**tx1, txs[0]);
  ASSERT_EQ(**tx2, txs[1]);
  ASSERT_EQ(blocks->getTxByHashSync(tx3hash), boost::none);
}

/**
 * @given initialized storage for ordering service
 * @when save proposal height
 * @then load proposal height and ensure it is correct
 */
TEST_F(AmetsuchiTest, OrderingServicePersistentStorageTest) {
  auto os_opt = storage->createOsPersistentState();
  ASSERT_TRUE(os_opt);
  auto ordering_state = os_opt.get();
  ASSERT_TRUE(ordering_state);

  ordering_state->resetState();
  ASSERT_EQ(2, ordering_state->loadProposalHeight().value());
  ASSERT_TRUE(ordering_state->saveProposalHeight(11));
  ASSERT_EQ(11, ordering_state->loadProposalHeight().value());
  ASSERT_TRUE(ordering_state->saveProposalHeight(33));
  ASSERT_EQ(33, ordering_state->loadProposalHeight().value());
  ordering_state->resetState();
  ASSERT_EQ(2, ordering_state->loadProposalHeight().value());
}

/**
 * @given initialized storage for ordering service
 * @when save proposal height
 * @then load proposal height and ensure it is correct
 */
TEST_F(AmetsuchiTest, OrderingServicePersistentStorageRestartTest) {
  auto os_opt = storage->createOsPersistentState();
  ASSERT_TRUE(os_opt);
  auto ordering_state = os_opt.get();
  ASSERT_TRUE(ordering_state);

  ordering_state->resetState();
  ASSERT_EQ(2, ordering_state->loadProposalHeight().value());
  ASSERT_TRUE(ordering_state->saveProposalHeight(11));
  ASSERT_EQ(11, ordering_state->loadProposalHeight().value());

  // restart Ordering Service Storage
  ordering_state.reset();
  os_opt = storage->createOsPersistentState();
  ASSERT_TRUE(os_opt);
  ordering_state = os_opt.get();
  ASSERT_TRUE(ordering_state);
  ASSERT_TRUE(ordering_state);
  ASSERT_EQ(11, ordering_state->loadProposalHeight().value());
}

/**
 * @given 2 different initialized storages for ordering service
 * @when save proposal height to the first one
 * @then the state is consistent
 */
TEST_F(AmetsuchiTest,
       OrderingServicePersistentStorageDifferentConnectionsTest) {
  auto os_opt1 = storage->createOsPersistentState();
  ASSERT_TRUE(os_opt1);
  auto ordering_state_1 = os_opt1.get();
  ASSERT_TRUE(ordering_state_1);

  auto os_opt2 = storage->createOsPersistentState();
  ASSERT_TRUE(os_opt2);
  auto ordering_state_2 = os_opt2.get();
  ASSERT_TRUE(ordering_state_2);

  ordering_state_2->resetState();
  ASSERT_EQ(2, ordering_state_1->loadProposalHeight().value());
  ASSERT_EQ(2, ordering_state_2->loadProposalHeight().value());
  ASSERT_TRUE(ordering_state_1->saveProposalHeight(11));
  ASSERT_EQ(11, ordering_state_1->loadProposalHeight().value());
  ASSERT_EQ(11, ordering_state_2->loadProposalHeight().value());

  ordering_state_2->resetState();
  ASSERT_EQ(2, ordering_state_1->loadProposalHeight().value());
  ASSERT_EQ(2, ordering_state_2->loadProposalHeight().value());
  ASSERT_TRUE(ordering_state_2->saveProposalHeight(42));
  ASSERT_EQ(42, ordering_state_1->loadProposalHeight().value());
  ASSERT_EQ(42, ordering_state_2->loadProposalHeight().value());
}

/**
 * @given spoiled WSV
 * @when WSV is restored
 * @then WSV is valid
 */
TEST_F(AmetsuchiTest, TestRestoreWSV) {
  // initialize storage with genesis block
  std::string default_domain = "test";
  std::string default_role = "admin";

  std::vector<shared_model::proto::Transaction> genesis_tx;
  genesis_tx.push_back(
      shared_model::proto::TransactionBuilder()
          .creatorAccountId("admin@test")
          .createdTime(iroha::time::now())
          .quorum(1)
          .createRole(default_role,
                      {Role::kCreateDomain,
                       Role::kCreateAccount,
                       Role::kAddAssetQty,
                       Role::kAddPeer,
                       Role::kReceive,
                       Role::kTransfer})
          .createDomain(default_domain, default_role)
          .build()
          .signAndAddSignature(
              shared_model::crypto::DefaultCryptoAlgorithmType::
                  generateKeypair())
          .finish());

  auto genesis_block = TestBlockBuilder()
                           .transactions(genesis_tx)
                           .height(1)
                           .prevHash(shared_model::crypto::Sha3_256::makeHash(
                               shared_model::crypto::Blob("")))
                           .createdTime(iroha::time::now())
                           .build();

  apply(storage, genesis_block);

  auto res = storage->getWsvQuery()->getDomain("test");
  EXPECT_TRUE(res);

  // spoil WSV
  *sql << "DELETE FROM domain";

  // check there is no data in WSV
  res = storage->getWsvQuery()->getDomain("test");
  EXPECT_FALSE(res);

  // recover storage and check it is recovered
  WsvRestorerImpl wsvRestorer;
  wsvRestorer.restoreWsv(*storage).match(
      [](iroha::expected::Value<void>) {},
      [&](iroha::expected::Error<std::string> &error) {
        FAIL() << "Failed to recover WSV";
      });

  res = storage->getWsvQuery()->getDomain("test");
  EXPECT_TRUE(res);
}

class PreparedBlockTest : public AmetsuchiTest {
 public:
  PreparedBlockTest()
      : key(shared_model::crypto::DefaultCryptoAlgorithmType::
                generateKeypair()) {}

  shared_model::proto::Transaction createAddAsset(const std::string &amount) {
    return shared_model::proto::TransactionBuilder()
        .creatorAccountId("admin@test")
        .createdTime(iroha::time::now())
        .quorum(1)
        .addAssetQuantity("coin#test", amount)
        .build()
        .signAndAddSignature(key)
        .finish();
  }

  shared_model::proto::Block createBlock(
      std::initializer_list<shared_model::proto::Transaction> txs) {
    return TestBlockBuilder()
        .transactions(std::vector<shared_model::proto::Transaction>(txs))
        .height(1)
        .prevHash(shared_model::crypto::Sha3_256::makeHash(
            shared_model::crypto::Blob("")))
        .createdTime(iroha::time::now())
        .build();
  }

  void SetUp() override {
    AmetsuchiTest::SetUp();
    genesis_tx =
        clone(shared_model::proto::TransactionBuilder()
                  .creatorAccountId("admin@test")
                  .createdTime(iroha::time::now())
                  .quorum(1)
                  .createRole(default_role,
                              {Role::kCreateDomain,
                               Role::kCreateAccount,
                               Role::kAddAssetQty,
                               Role::kAddPeer,
                               Role::kReceive,
                               Role::kTransfer})
                  .createDomain(default_domain, default_role)
                  .createAccount("admin", "test", key.publicKey())
                  .createAsset("coin", default_domain, 2)
                  .addAssetQuantity("coin#test", base_balance.toStringRepr())
                  .build()
                  .signAndAddSignature(key)
                  .finish());
    genesis_block = clone(createBlock({*genesis_tx}));
    initial_tx = clone(createAddAsset("5.00"));
    apply(storage, *genesis_block);
    using framework::expected::val;
    temp_wsv = std::move(val(storage->createTemporaryWsv())->value);
  }

  shared_model::crypto::Keypair key;
  std::string default_domain{"test"};
  std::string default_role{"admin"};
  std::unique_ptr<shared_model::proto::Transaction> genesis_tx;
  std::unique_ptr<shared_model::proto::Transaction> initial_tx;
  std::unique_ptr<shared_model::proto::Block> genesis_block;
  std::unique_ptr<iroha::ametsuchi::TemporaryWsv> temp_wsv;
  shared_model::interface::Amount base_balance{"5.00"};
};

/**
 * @given TemporaryWSV with several transactions
 * @when block is prepared for two phase commit
 * @then state of the ledger remains unchanged
 */
TEST_F(PreparedBlockTest, PrepareBlockNoStateChanged) {
  validateAccountAsset(storage->getWsvQuery(),
                       "admin@test",
                       "coin#test",
                       shared_model::interface::Amount(base_balance));

  auto result = temp_wsv->apply(*initial_tx);
  ASSERT_FALSE(framework::expected::err(result));
  storage->prepareBlock(std::move(temp_wsv));

  // balance remains unchanged
  validateAccountAsset(
      storage->getWsvQuery(), "admin@test", "coin#test", base_balance);
}

/**
 * @given Storage with prepared state
 * @when prepared state is applied
 * @then state of the ledger is changed
 */
TEST_F(PreparedBlockTest, CommitPreparedStateChanged) {
  shared_model::interface::Amount added_amount{"5.00"};

  auto block = createBlock({*initial_tx});

  auto result = temp_wsv->apply(*initial_tx);
  ASSERT_FALSE(framework::expected::err(result));
  storage->prepareBlock(std::move(temp_wsv));

  auto commited = storage->commitPrepared(block);

  EXPECT_TRUE(commited);

  shared_model::interface::Amount resultingAmount("10.00");

  validateAccountAsset(
      storage->getWsvQuery(), "admin@test", "coin#test", resultingAmount);
}

/**
 * @given Storage with prepared state
 * @when another block is applied
 * @then state of the ledger is changed to that of the applied block
 * and not of the prepared state
 */
TEST_F(PreparedBlockTest, PrepareBlockCommitDifferentBlock) {
  // tx which actually gets commited
  auto other_tx = createAddAsset("10.00");

  auto block = createBlock({other_tx});

  auto result = temp_wsv->apply(*initial_tx);
  ASSERT_TRUE(framework::expected::val(result));
  storage->prepareBlock(std::move(temp_wsv));

  apply(storage, block);

  shared_model::interface::Amount resultingBalance{"15.00"};
  validateAccountAsset(
      storage->getWsvQuery(), "admin@test", "coin#test", resultingBalance);
}

/**
 * @given Storage with prepared state
 * @when another block is applied
 * @then commitPrepared fails @and prepared state is not applied
 */
TEST_F(PreparedBlockTest, CommitPreparedFailsAfterCommit) {
  // tx which we prepare
  auto tx = createAddAsset("5.00");

  // tx which actually gets commited
  auto other_tx = createAddAsset("10.00");

  auto block = createBlock({other_tx});

  auto result = temp_wsv->apply(*initial_tx);
  ASSERT_FALSE(framework::expected::err(result));
  storage->prepareBlock(std::move(temp_wsv));

  apply(storage, block);

  auto commited = storage->commitPrepared(block);

  ASSERT_FALSE(commited);

  shared_model::interface::Amount resultingBalance{"15.00"};
  validateAccountAsset(
      storage->getWsvQuery(), "admin@test", "coin#test", resultingBalance);
}
