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
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
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
using iroha::model::Transaction;
using iroha::model::Command;
using iroha::model::generators::BlockGenerator;
using iroha::model::generators::TransactionGenerator;
using iroha::model::generators::CommandGenerator;

static const auto ADMIN_ID = std::string("admin@test");
static const auto DOMAIN_ID = std::string("domain");
static const auto USER_DEFAULT_ROLE = std::string("user");
static const auto ALICE_NAME = std::string("alice");
static const auto ALICE_ID = ALICE_NAME + "@" + DOMAIN_ID;
static const auto BOB_NAME = std::string("bob");
static const auto BOB_ID = BOB_NAME + "@" + DOMAIN_ID;
static const auto IRH_ASSET_NAME = std::string("irh");
static const auto IRH_ASSET_ID = IRH_ASSET_NAME + "#" + DOMAIN_ID;
static const auto MEK_ASSET_NAME = std::string("moeka");
static const auto MEK_ASSET_ID = MEK_ASSET_NAME + "#" + DOMAIN_ID;

/**
 * StorageImpl
 * genesis_block
 *   CreateRole, CreateDomain(domain), CreateAccount(alice@domain, bob@domain),
 *   CreateAsset(irh#domain), CreateAsset(moeka#domain)
 * block1:
 *   given_txs[0]: creator_account_id = admin@domain
 *     - AddAssetQuantity(alice@domain, irh#domain, 123.4)
 *   given_txs[1]: creator_account_id = admin@domain
 *     - AddAssetQuantity(bob@domain, moeka#domain, 100.50)
 * block2:
 *   given_txs[2]: creator_account_id = admin@domain
 *     - TransferAsset(src: alice@domain, dest: bob@domain, irh#domain, 23.4)
 *   given_txs[3]: creator_account_id = admin@domain
 *     - random command
 *     - TransferAsset(src: bob@domain, dest: alice@domain, moeka#domain, 20.00)
 *     - random command
 */
class GetAccountAssetTransactionsTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();
    storage =
        StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
    ASSERT_TRUE(storage);

    blocks = storage->getBlockQuery();

    const auto genesis_block = BlockGenerator().generateBlock(
        0, 1, iroha::hash256_t{}, {[] {
          return TransactionGenerator().generateTransaction(
              ADMIN_ID,
              0,
              {std::make_shared<iroha::model::CreateRole>(
                   USER_DEFAULT_ROLE, iroha::model::all_perm_group),
               CommandGenerator().generateCreateDomain(DOMAIN_ID,
                                                       USER_DEFAULT_ROLE),
               CommandGenerator().generateCreateAccount(
                   ALICE_NAME,
                   DOMAIN_ID,
                   generator::random_blob<iroha::pubkey_t::size()>(1)),
               CommandGenerator().generateCreateAccount(
                   BOB_NAME,
                   DOMAIN_ID,
                   generator::random_blob<iroha::pubkey_t::size()>(2)),
               CommandGenerator().generateCreateAsset(
                   IRH_ASSET_NAME, DOMAIN_ID, 1),
               CommandGenerator().generateCreateAsset(
                   MEK_ASSET_NAME, DOMAIN_ID, 2)});
        }()});
    ASSERT_TRUE(storage->insertBlock(genesis_block));

    given_txs.emplace_back(TransactionGenerator().generateTransaction(
        ADMIN_ID,
        0,
        {CommandGenerator().generateAddAssetQuantity(
            ALICE_ID,
            IRH_ASSET_ID,
            *iroha::Amount::createFromString("123.4"))}));
    given_txs.emplace_back(TransactionGenerator().generateTransaction(
        ADMIN_ID,
        1,
        {CommandGenerator().generateAddAssetQuantity(
            BOB_ID,
            MEK_ASSET_ID,
            *iroha::Amount::createFromString("100.50"))}));

    const auto block1 = BlockGenerator().generateBlock(
        0, 2, genesis_block.hash, {given_txs[0], given_txs[1]});
    ASSERT_TRUE(storage->insertBlock(block1));

    given_txs.emplace_back(TransactionGenerator().generateTransaction(
        ADMIN_ID,
        0,
        {CommandGenerator().generateTransferAsset(
            ALICE_ID,
            BOB_ID,
            IRH_ASSET_ID,
            *iroha::Amount::createFromString("23.4"))}));
    given_txs.emplace_back(TransactionGenerator().generateTransaction(
        ADMIN_ID,
        0,
        {generate_random_command(),
         CommandGenerator().generateTransferAsset(
             BOB_ID,
             ALICE_ID,
             MEK_ASSET_ID,
             *iroha::Amount::createFromString("20.00")),
         generate_random_command()}));
    const auto block2 = BlockGenerator().generateBlock(
        0, 3, block1.hash, {given_txs[2], given_txs[3]});
    ASSERT_TRUE(storage->insertBlock(block2));
  }

  std::shared_ptr<Command> generate_random_command() {
    return CommandGenerator().generateCreateDomain(
        generator::random_string(20, generator::random_lower_char), "user");
  }

  std::vector<Transaction> given_txs;
  std::shared_ptr<StorageImpl> storage;
  std::shared_ptr<BlockQuery> blocks;
};

/**
 * @brief No transactions when pager.limit = 0.
 *
 * @given StorageImpl
 * @when GetAccountAssetTxs(account_id: alice@domain1, asset_id: [irh#domain],
 *       pager: {tx_hash: {}, limit: 0})
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountAssetTransactionsTest, NoTxsWhenPagerLimitIs0) {
  const auto pager = Pager{iroha::hash256_t{}, 0};

  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountAssetTransactions(ALICE_ID, {IRH_ASSET_ID}, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Parts of matched transactions when pager.limit specified.
 *
 * @given StorageImpl
 * @when (account_id: alice@domain1, asset_id: [irh#domain], pager: {tx_hash:
 * {}, limit: 1})
 * @then Transactions [2] can be retrieved.
 */
TEST_F(GetAccountAssetTransactionsTest, PartsOfTxsWhenPagerLimit) {
  const auto pager = Pager{iroha::hash256_t{}, 1};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountAssetTransactions(ALICE_ID, {IRH_ASSET_ID}, pager),
      std::vector<Transaction>{given_txs[2]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief All transactions when num of inserted txs less than pager.limit.
 *
 * @given StorageImpl
 * @when (account_id: alice@domain1, asset_id: [irh#domain], pager: {tx_hash:
 * {}, limit: MAX_PAGER_LIMIT})
 * @then All matched transactions can be retrieved.
 */
TEST_F(GetAccountAssetTransactionsTest,
       AllTxsWhenInsertedTxsIsLessThanPagerLimit) {
  const auto pager = Pager{iroha::hash256_t{}, Pager::MAX_PAGER_LIMIT};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountAssetTransactions(ALICE_ID, {IRH_ASSET_ID}, pager),
      std::vector<Transaction>{given_txs[2], given_txs[0]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Transactions when specified multiple asset id related to account id.
 *
 * @given StorageImpl
 * @when (account_id: alice@domain1, asset_id: [irh#domain, moeka#domain],
 *        pager: {tx_hash: {}, limit: MAX_PAGER_LIMIT})
 * @then All matched transactions can be retrieved.
 * @note account id matches either txs which have TransferAsset's source or
 * destination id.
 */
TEST_F(GetAccountAssetTransactionsTest, MultipleAssetId) {
  const auto pager = Pager{iroha::hash256_t{}, Pager::MAX_PAGER_LIMIT};

  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountAssetTransactions(
          ALICE_ID, {IRH_ASSET_ID, MEK_ASSET_ID}, pager),
      std::vector<Transaction>{given_txs[3], given_txs[2], given_txs[0]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Transactions with pager.tx_hash
 *
 * @given StorageImpl
 * @when (account_id: alice@domain1, asset_id: [irh#domain1],
 * pager: {tx_hash: hash(given_txs[2]), limit: MAX_PAGER_LIMIT})
 * @then Transaction 0 can be retrieved.
 * @note The transaction of tx_hash is excluded.
 *       Retrieving transactions from newer to older transactions.
 */
TEST_F(GetAccountAssetTransactionsTest, SpecificPagerTxHash) {
  const auto pager = Pager{iroha::hash(given_txs[2]), Pager::MAX_PAGER_LIMIT};

  auto wrapper1 = make_test_subscriber<EqualToList>(
      blocks->getAccountAssetTransactions(ALICE_ID, {IRH_ASSET_ID}, pager),
      std::vector<Transaction>{given_txs[0]});
  ASSERT_TRUE(wrapper1.subscribe().validate());
}

/**
 * @brief No transactions with empty asset id vector.
 *
 * @given StorageImpl
 * @when (account_id: alice@domain1, asset_id: [],
 * pager: {tx_hash: {}, limit: MAX_PAGER_LIMIT})
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountAssetTransactionsTest, EmptyAssetId) {
  const auto pager = Pager{iroha::hash256_t{}, Pager::MAX_PAGER_LIMIT};

  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountAssetTransactions(ALICE_ID, {}, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief No transactions with empty storage.
 *
 * @given Empty StorageImpl
 * @when some query
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountAssetTransactionsTest, EmptyStorage) {
  storage->dropStorage();
  const auto pager = Pager{iroha::hash256_t{}, Pager::MAX_PAGER_LIMIT};

  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountAssetTransactions(ALICE_ID, {IRH_ASSET_ID}, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}
