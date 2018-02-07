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

/**
 * @brief This test is an integration test of BlockQuery with {RedisBlockIndex,
 * RedisBlockQuery} and BlockStorage with {BlockStorageNuDB}
 * @note depends on started Ametsuchi (Redis + Postgres)
 */

#include "ametsuchi/impl/block_storage_nudb.hpp"
#include "block_query_fixture.hpp"
#include "framework/test_subscriber.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

class BlockQueryWithNuDB : public BlockQueryFixture {
 public:
  void SetUp() override {
    BlockQueryFixture::SetUp();

    auto bs = BlockStorageNuDB::create(block_store_path);
    ASSERT_TRUE(bs) << "block storage failed";
    bs_ = std::move(*bs);

    index = std::make_shared<RedisBlockIndex>(client);
    query = std::make_shared<RedisBlockQuery>(client, *bs_);

    for (const auto &b : blocks) {
      auto id = static_cast<BlockStorage::Identifier>(b.height);
      std::vector<uint8_t> data =
          iroha::stringToBytes(converters::jsonToString(conv->serialize(b)));

      bs_->add(id, data);
      index->index(b);
    }
  }

  std::string block_store_path = "/tmp/iroha_block_storage";
};

///////////////////////////////////////////////////////////////////////
/// BlockQueryWithNuDB

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions created by user1@test is invoked
 * @then query over user1@test returns 3 txs
 */
TEST_F(BlockQueryWithNuDB, GetAccountTransactionsFromSeveralBlocks) {
  // Check that creator1 has created 3 transactions
  auto getCreator1TxWrapper = make_test_subscriber<CallExact>(
      query->getAccountTransactions(creator1), 3);
  getCreator1TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(val.creator_account_id, creator1); });
  ASSERT_TRUE(getCreator1TxWrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions created by user2@test is invoked
 * @then query over user2@test returns 1 tx
 */
TEST_F(BlockQueryWithNuDB, GetAccountTransactionsFromSingleBlock) {
  // Check that creator1 has created 1 transaction
  auto getCreator2TxWrapper = make_test_subscriber<CallExact>(
      query->getAccountTransactions(creator2), 1);
  getCreator2TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(val.creator_account_id, creator2); });
  ASSERT_TRUE(getCreator2TxWrapper.validate());
}

/**
 * @given block store
 * @when query to get transactions created by user with id not registered in the
 * system is invoked
 * @then query returns empty result
 */
TEST_F(BlockQueryWithNuDB, GetAccountTransactionsNonExistingUser) {
  // Check that "nonexisting" user has no transaction
  auto getNonexistingTxWrapper = make_test_subscriber<CallExact>(
      query->getAccountTransactions("nonexisting user"), 0);
  getNonexistingTxWrapper.subscribe();
  ASSERT_TRUE(getNonexistingTxWrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with existing transaction hashes
 * @then queried transactions
 */
TEST_F(BlockQueryWithNuDB, GetTransactionsExistingTxHashes) {
  auto wrapper = make_test_subscriber<CallExact>(
      query->getTransactions({tx_hashes[1], tx_hashes[3]}), 2);
  wrapper.subscribe([this](auto tx) {
    static auto subs_cnt = 0;
    subs_cnt++;
    if (subs_cnt == 1) {
      EXPECT_TRUE(tx);
      EXPECT_EQ(tx_hashes[1], iroha::hash(*tx));
    } else {
      EXPECT_TRUE(tx);
      EXPECT_EQ(tx_hashes[3], iroha::hash(*tx));
    }
  });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with non-existing transaction hashes
 * @then nullopt values are retrieved
 */
TEST_F(BlockQueryWithNuDB, GetTransactionsIncludesNonExistingTxHashes) {
  iroha::hash256_t invalid_tx_hash_1, invalid_tx_hash_2;
  invalid_tx_hash_1[0] = 1;
  invalid_tx_hash_2[0] = 2;
  auto wrapper = make_test_subscriber<CallExact>(
      query->getTransactions({invalid_tx_hash_1, invalid_tx_hash_2}), 2);
  wrapper.subscribe(
      [](auto transaction) { EXPECT_EQ(boost::none, transaction); });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with empty vector
 * @then no transactions are retrieved
 */
TEST_F(BlockQueryWithNuDB, GetTransactionsWithEmpty) {
  // transactions' hashes are empty.
  auto wrapper = make_test_subscriber<CallExact>(query->getTransactions({}), 0);
  wrapper.subscribe();
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with non-existing txhash and existing txhash
 * @then queried transactions and empty transaction
 */
TEST_F(BlockQueryWithNuDB, GetTransactionsWithInvalidTxAndValidTx) {
  // TODO 15/11/17 motxx - Use EqualList VerificationStrategy
  iroha::hash256_t invalid_tx_hash_1;
  invalid_tx_hash_1[0] = 1;
  auto wrapper = make_test_subscriber<CallExact>(
      query->getTransactions({invalid_tx_hash_1, tx_hashes[0]}), 2);
  wrapper.subscribe([this](auto tx) {
    static auto subs_cnt = 0;
    subs_cnt++;
    if (subs_cnt == 1) {
      EXPECT_EQ(boost::none, tx);
    } else {
      EXPECT_TRUE(tx);
      EXPECT_EQ(tx_hashes[0], iroha::hash(*tx));
    }
  });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get non-existent 1000th block
 * @then nothing is returned
 */
TEST_F(BlockQueryWithNuDB, GetNonExistentBlock) {
  auto wrapper = make_test_subscriber<CallExact>(query->getBlocks(1000, 1), 0);
  wrapper.subscribe();
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when height=1, count=1
 * @then returned exactly 1 block
 */
TEST_F(BlockQueryWithNuDB, GetExactlyOneBlock) {
  auto wrapper = make_test_subscriber<CallExact>(query->getBlocks(1, 1), 1);
  wrapper.subscribe();
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when count=0
 * @then no blocks returned
 */
TEST_F(BlockQueryWithNuDB, GetBlocks_Count0) {
  auto wrapper = make_test_subscriber<CallExact>(query->getBlocks(1, 0), 0);
  wrapper.subscribe();
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get all blocks starting from 0
 * @then returned all blocks (2)
 */
TEST_F(BlockQueryWithNuDB, GetBlocksFrom0) {
  auto wrapper = make_test_subscriber<CallExact>(
      query->getBlocksFrom(AmetsuchiTest::FIRST_BLOCK), blocks.size());
  size_t counter = AmetsuchiTest::FIRST_BLOCK;
  wrapper.subscribe([&counter](Block b) {
    // wrapper returns blocks 1 and 2
    ASSERT_EQ(b.height, counter++) << "block height: " << b.height
                                   << "counter: " << counter;
  });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test)
 * @when get top 2 blocks
 * @then last 2 blocks returned with correct height
 */
TEST_F(BlockQueryWithNuDB, GetTop2Blocks) {
  size_t blocks_n = 2;  // top 2 blocks
  auto wrapper =
      make_test_subscriber<CallExact>(query->getTopBlocks(blocks_n), blocks_n);

  size_t counter = blocks.size() - blocks_n;
  wrapper.subscribe([&counter](Block b) { ASSERT_EQ(b.height, counter++); });

  ASSERT_TRUE(wrapper.validate());
}
