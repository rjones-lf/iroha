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

#include <boost/optional.hpp>
#include "ametsuchi/impl/storage_impl.hpp"
#include "crypto/hash.hpp"
#include "framework/test_subscriber.hpp"
#include "generator/generator.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/create_role.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/generators/block_generator.hpp"
#include "model/generators/transaction_generator.hpp"
#include "model/permissions.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

using model::generators::BlockGenerator;

static const auto NO_PAGER = iroha::model::Pager{iroha::hash256_t{}, 10000};

class BlockQueryTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();
    storage =
        StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);

    ASSERT_TRUE(storage);

    auto genesis_tx = iroha::model::Transaction{};
    tx.creator_account_id = "default@defaultdomain",
    tx.commands = {std::make_shared<iroha::model::CreateRole>(
                       DOMAIN_USER_DEFAULT_ROLE, iroha::model::all_perm_group),
                   std::make_shared<iroha::model::CreateDomain>(
                       DOMAIN_ID, DOMAIN_USER_DEFAULT_ROLE),
                   std::make_shared<iroha::model::CreateAccount>(
                       ALICE_NAME, DOMAIN_ID, iroha::pubkey_t{}),
                   std::make_shared<iroha::model::CreateAccount>(
                       BOB_NAME, DOMAIN_ID, iroha::pubkey_t{}),
                   std::make_shared<iroha::model::CreateAsset>(
                       ASSET1_NAME, DOMAIN_ID, ASSET1_PREC),
                   std::make_shared<iroha::model::CreateAsset>(
                       ASSET2_NAME, DOMAIN_ID, ASSET2_PREC)};

    const auto genesis_block =
        BlockGenerator().generateBlock(0, 1, iroha::hash256_t{}, {genesis_tx});
    EXPECT_TRUE(storage->insertBlock(genesis_block));

    const auto tx1 = make_tx(ALICE_ID);
    const auto tx2 = make_tx(ALICE_ID);
    const auto block1 =
        BlockGenerator().generateBlock(0, 2, genesis_block.hash, {tx1, tx2});
    ASSERT_TRUE(storage->insertBlock(block1));

    const auto tx3 = make_tx(BOB_ID);
    const auto tx4 = make_tx(ALICE_ID);
    const auto block2 =
        BlockGenerator().generateBlock(0, 3, block1.hash, {tx3, tx4});
    ASSERT_TRUE(storage->insertBlock(block2));
  }

  std::vector<iroha::hash256_t> tx_hashes;
  std::shared_ptr<StorageImpl> storage;
  std::shared_ptr<BlockQuery> blocks;
  std::string creator1 = "user1@test";
  std::string creator2 = "user2@test";

  const auto DOMAIN_ID = std::string("domain");
  const auto DOMAIN_USER_DEFAULT_ROLE = "user";
  const auto ALICE_NAME = std::string("alice");
  const auto BOB_NAME = std::string("bob");
  const auto ALICE_ID = ALICE_NAME + "@" + DOMAIN_ID;
  const auto BOB_ID = BOB_NAME + "@" + DOMAIN_ID;
};

/**
 * @brief No transactions when pager.limit = 0.
 *
 * @given StorageImpl inserted a transaction:
 *   1. creator_account_id = alice@domain
 * @when Query alice's transactions with pager.limit = 0
 * @then No transactions can be retrieved.
 */
TEST_F(BlockQueryTest, NoTxsWhenGetAcctTxsPagerLimit0) {
  const auto pager = Pager{iroha::hash256_t{}, 0};
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions(ALICE_ID, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Skip other creator transactions.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = bob@domain
 *   tx3: creator_account_id = alice@domain
 * @when Query alice[bob]'s transactions with pager.limit = 2[1]
 * @then Alice[bob]'s transactions [3, 1]/[2] can be retrieved.
 */
TEST_F(BlockQueryTest, SkipOtherCreatorWhenGetAcctTxsPagerLimit1) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(BOB_ID);
  const auto tx3 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2, tx3}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  const auto pager1 = Pager{iroha::hash256_t{}, 2};
  auto wrapper1 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager1),
      std::vector<Transaction>{tx3, tx1});
  ASSERT_TRUE(wrapper1.subscribe().validate());

  const auto pager2 = Pager{iroha::hash256_t{}, 1};
  auto wrapper2 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(BOB_ID, pager2),
      std::vector<Transaction>{tx2});
  ASSERT_TRUE(wrapper2.subscribe().validate());
}

/**
 * @brief Valid when num of inserted txs in storage less than pager.limit.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = alice@domain
 * @when Query alice's transactions with pager.limit = 100
 * @then All alice's transactions [2, 1] can be retrieved.
 */
TEST_F(BlockQueryTest, AllTxsWhenGetAcctTxsInsNumLessThanPagerLimit) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  const auto pager = Pager{iroha::hash256_t{}, 100};
  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{tx2, tx1});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Valid when retrievable txs is in multiple blocks.
 *
 * @given StorageImpl inserted transactions in multiple blocks:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = alice@domain
 *   tx3: creator_account_id = bob@domain
 *   tx4: creator_account_id = alice@domain
 * @when Query alice's transactions with pager.limit = 100
 * @then All alice's transactions in blocks
 */
TEST_F(BlockQueryTest, ValidTxsInMultipleBlocksWhenGetAcctTxs) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(ALICE_ID);
  const auto block1 = make_block({tx1, tx2}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block1));

  const auto tx3 = make_tx(BOB_ID);
  const auto tx4 = make_tx(ALICE_ID);
  const auto block2 = make_block({tx3, tx4}, 3, block1.hash);
  ASSERT_TRUE(storage->insertBlock(block2));

  const auto pager = Pager{iroha::hash256_t{}, 100};

  auto wrapper1 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{tx4, tx2, tx1});
  ASSERT_TRUE(wrapper1.subscribe().validate());

  auto wrapper2 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(BOB_ID, pager),
      std::vector<Transaction>{tx3});
  ASSERT_TRUE(wrapper2.subscribe().validate());
}

/**
 * @brief Valid when the tx of pager.tx_hash belongs to the query's creator.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = alice@domain
 * @when Query alice's transactions with pager{tx_hash: tx2, limit: 100}
 * @then One transaction 1 can be retrieved.
 *
 * @note A tx which corresponds to pager.tx_hash is excluded in response.
 */
TEST_F(BlockQueryTest, GetAcctTxsWithPagerHash) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  const auto pager = Pager{iroha::hash(tx2), 100};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{tx1});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Valid when the tx of pager.tx_hash doesn't belong to the query's
 * creator.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = bob@domain
 *   tx3: creator_account_id = alice@domain
 * @when Query alice's transactions with pager{tx_hash: tx2, limit: 100}
 * @then One transaction 1 can be retrieved.
 *
 * @note A tx which corresponds to pager.tx_hash is excluded in response.
 */
TEST_F(BlockQueryTest, GetAcctTxsWithPagerOtherCreatorHash) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(BOB_ID);
  const auto tx3 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2, tx3}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  const auto pager = Pager{iroha::hash(tx2), 100};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{tx1});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Regards pager.tx_hash as empty when the hash is invalid.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = alice@domain
 * @when Query alice's transactions with pager{tx_hash: invalid bytes, limit:
 * 100}
 * @then Regards pager.tx_hash as empty and transactions [2, 1] can be
 * retrieved.
 *
 * @note Stateful validation will fail when this query sent.
 */
TEST_F(BlockQueryTest, RegardsTxHashAsEmptyWhenGetAcctTxsWithInvalidHash) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  iroha::hash256_t invalid_hash;
  invalid_hash.at(0) = 1;
  const auto pager = Pager{invalid_hash, 100};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{tx2, tx1});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief No transactions when the query's creator is not found.
 *
 * @given StorageImpl inserted transactions:
 *   tx1: creator_account_id = alice@domain
 *   tx2: creator_account_id = alice@domain
 * @when Query none@somewhere creator's transactions with pager.limit = 100
 * @then No transactions can be retrieved.
 */
TEST_F(BlockQueryTest, NoTxsWhenGetAcctTxsWithInvalidCreator) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();
  const auto default_block_hash = insert_default_block(storage);

  const auto tx1 = make_tx(ALICE_ID);
  const auto tx2 = make_tx(ALICE_ID);
  const auto block = make_block({tx1, tx2}, 2, default_block_hash);
  ASSERT_TRUE(storage->insertBlock(block));

  const auto no_account = "none@somewhere";
  const auto pager = Pager{iroha::hash256_t{}, 100};

  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions(no_account, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief No transactions when the storage is empty.
 *
 * @given Empty StorageImpl
 * @when Query with pager.limit = 100
 * @then No transactions can be retrieved.
 */
TEST_F(BlockQueryTest, NoTxsWhenGetAcctTxsToEmptyStorage) {
  using namespace default_block;
  const auto storage =
      StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
  ASSERT_TRUE(storage);
  const auto blocks = storage->getBlockQuery();

  blocks->getTopBlocks(1).subscribe(
      [](auto blk) { FAIL() << "Storage must be empty."; });

  const auto pager = Pager{iroha::hash256_t{}, 100};

  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions("alice@domain", pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}
