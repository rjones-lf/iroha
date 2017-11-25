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
using iroha::model::Transaction;
using iroha::model::generators::BlockGenerator;
using iroha::model::generators::TransactionGenerator;
using iroha::model::generators::CommandGenerator;

static const auto NO_PAGER = iroha::model::Pager{iroha::hash256_t{}, 10000};

static const auto DOMAIN_ID = std::string("domain");
static const auto DOMAIN_USER_DEFAULT_ROLE = std::string("user");
static const auto ALICE_NAME = std::string("alice");
static const auto BOB_NAME = std::string("bob");
static const auto ALICE_ID = ALICE_NAME + "@" + DOMAIN_ID;
static const auto BOB_ID = BOB_NAME + "@" + DOMAIN_ID;

class GetAccountTransactionsTest : public AmetsuchiTest {
 protected:
  /**
   * StorageImpl
   *   1. block1 (genesis block)
   *     - Create accounts alice@domain, bob@domain
   *   2. block2
   *     - given_txs[0]: creator_account_id = alice@domain
   *     - given_txs[1]: creator_account_id = alice@domain
   *   3. block3
   *     - given_txs[2]: creator_account_id = bob@domain
   *     - given_txs[3]: creator_account_id = alice@domain
   */
  void SetUp() override {
    AmetsuchiTest::SetUp();
    storage =
        StorageImpl::create(block_store_path, redishost_, redisport_, pgopt_);
    ASSERT_TRUE(storage);

    blocks = storage->getBlockQuery();

    const auto genesis_tx = TransactionGenerator().generateTransaction(
        "default@defaultdomain",
        0,
        {std::make_shared<iroha::model::CreateRole>(
             DOMAIN_USER_DEFAULT_ROLE, iroha::model::all_perm_group),
         CommandGenerator().generateCreateDomain(DOMAIN_ID,
                                                 DOMAIN_USER_DEFAULT_ROLE),
         CommandGenerator().generateCreateAccount(
             ALICE_NAME, DOMAIN_ID, iroha::pubkey_t{}),
         CommandGenerator().generateCreateAccount(
             BOB_NAME, DOMAIN_ID, iroha::pubkey_t{})});

    const auto genesis_block =
        BlockGenerator().generateBlock(0, 1, iroha::hash256_t{}, {genesis_tx});
    EXPECT_TRUE(storage->insertBlock(genesis_block));

    given_txs.emplace_back(generate_random_tx(ALICE_ID));
    given_txs.emplace_back(generate_random_tx(ALICE_ID));
    const auto block1 = BlockGenerator().generateBlock(
        0, 2, genesis_block.hash, {given_txs[0], given_txs[1]});
    ASSERT_TRUE(storage->insertBlock(block1));

    given_txs.emplace_back(generate_random_tx(BOB_ID));
    given_txs.emplace_back(generate_random_tx(ALICE_ID));
    const auto block2 = BlockGenerator().generateBlock(
        0, 3, block1.hash, {given_txs[2], given_txs[3]});
    ASSERT_TRUE(storage->insertBlock(block2));
  }

  Transaction generate_random_tx(
      const std::string &creator,
      const std::vector<std::shared_ptr<iroha::model::Command>> &commands = {
          CommandGenerator().generateCreateDomain(
              generator::random_string(20, generator::random_lower_char),
              "user")}) {
    return TransactionGenerator().generateTransaction(
        creator,
        static_cast<uint64_t>(generator::random_number(0, 1000)),
        commands);
  }

  std::vector<Transaction> given_txs;
  std::shared_ptr<StorageImpl> storage;
  std::shared_ptr<BlockQuery> blocks;
};

/**
 * @brief No transactions when pager.limit = 0.
 * @given StorageImpl
 * @when Query alice's transactions with pager.limit = 0
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountTransactionsTest, NoTxsWhenGetAcctTxsPagerLimit0) {
  const auto pager = Pager{iroha::hash256_t{}, 0};
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions(ALICE_ID, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Skip other creator transactions with pager.limit.
 * @given StorageImpl
 * @when Query alice[bob]'s transactions with pager.limit = 2, 1
 * @then Alice[bob]'s transactions [3, 1], [2] can be retrieved.
 */
TEST_F(GetAccountTransactionsTest, SkipOtherCreatorWhenGetAcctTxsPagerLimit1) {
  const auto pager1 = Pager{iroha::hash256_t{}, 2};
  auto wrapper1 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager1),
      std::vector<Transaction>{given_txs[3], given_txs[1]});
  ASSERT_TRUE(wrapper1.subscribe().validate());

  const auto pager2 = Pager{iroha::hash256_t{}, 1};
  auto wrapper2 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(BOB_ID, pager2),
      std::vector<Transaction>{given_txs[2]});
  ASSERT_TRUE(wrapper2.subscribe().validate());
}

/**
 * @brief Valid when num of inserted txs in storage less than pager.limit.
 * @given StorageImpl
 * @when Query alice's transactions with pager.limit = 100
 * @then Alice's transactions can be retrieved.
 */
TEST_F(GetAccountTransactionsTest,
       AllTxsWhenGetAcctTxsInsNumLessThanPagerLimit) {
  const auto pager = Pager{iroha::hash256_t{}, 100};
  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{given_txs[3], given_txs[1], given_txs[0]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Validate a transaction specified by tx_hash is excluded.
 * @given StorageImpl
 * @when Query alice's transactions with pager{tx_hash: tx2, limit: 100}
 * @then Alice's transactions [1, 0] can be retrieved.
 */
TEST_F(GetAccountTransactionsTest, GetAcctTxsWithPagerHash) {
  const auto pager = Pager{iroha::hash(given_txs[3]), 100};
  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{given_txs[1], given_txs[0]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief Valid when the tx of pager.tx_hash doesn't belong to the query's
 * creator.
 * @given StorageImpl
 * @when Query alice's transactions with pager{tx_hash: tx2, limit: 100}
 * @then One transaction 1 can be retrieved.
 *
 * @note A tx which corresponds to pager.tx_hash is excluded in response.
 */
TEST_F(GetAccountTransactionsTest, GetAcctTxsWithPagerOtherCreatorHash) {
  const auto pager = Pager{iroha::hash(given_txs[2]), 100};
  auto wrapper1 = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{given_txs[1], given_txs[0]});
  ASSERT_TRUE(wrapper1.subscribe().validate());
}

/**
 * @brief Regards pager.tx_hash as empty when the hash is invalid.
 * @given StorageImpl
 * @when Query alice's transactions with pager{tx_hash: invalid bytes, limit:
 * 100}
 * @then Regards pager.tx_hash as empty and transactions [3, 2, 1] can be
 * retrieved.
 */
TEST_F(GetAccountTransactionsTest,
       RegardsTxHashAsEmptyWhenGetAcctTxsWithInvalidHash) {
  iroha::hash256_t invalid_hash;
  invalid_hash.at(0) = 1;
  const auto pager = Pager{invalid_hash, 100};
  auto wrapper = make_test_subscriber<EqualToList>(
      blocks->getAccountTransactions(ALICE_ID, pager),
      std::vector<Transaction>{given_txs[3], given_txs[1], given_txs[0]});
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief No transactions when the query's creator is not found.
 * @given StorageImpl
 * @when Query none@somewhere creator's transactions with pager.limit = 100
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountTransactionsTest, NoTxsWhenGetAcctTxsWithInvalidCreator) {
  const auto no_account = "none@somewhere";
  const auto pager = Pager{iroha::hash256_t{}, 100};
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions(no_account, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}

/**
 * @brief No transactions when the storage is empty.
 * @given Empty StorageImpl
 * @when Query Alice's transactions with pager.limit = 100
 * @then No transactions can be retrieved.
 */
TEST_F(GetAccountTransactionsTest, NoTxsWhenGetAcctTxsToEmptyStorage) {
  storage->dropStorage();
  const auto pager = Pager{iroha::hash256_t{}, 100};
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getAccountTransactions(ALICE_ID, pager), 0);
  ASSERT_TRUE(wrapper.subscribe().validate());
}
