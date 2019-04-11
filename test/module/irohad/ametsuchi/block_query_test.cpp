/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_block_query.hpp"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "ametsuchi/impl/postgres_block_index.hpp"
#include "backend/protobuf/proto_block_json_converter.hpp"
#include "common/byteutils.hpp"
#include "converters/protobuf/json_proto_converter.hpp"
#include "framework/result_fixture.hpp"
#include "framework/test_logger.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/irohad/ametsuchi/mock_key_value_storage.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace iroha::ametsuchi;

using testing::Return;

class BlockQueryTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();

    auto tmp = FlatFile::create(block_store_path, getTestLogger("FlatFile"));
    ASSERT_TRUE(tmp);
    file = std::move(*tmp);
    mock_file = std::make_shared<MockKeyValueStorage>();
    sql = std::make_unique<soci::session>(*soci::factory_postgresql(), pgopt_);

    index =
        std::make_shared<PostgresBlockIndex>(*sql, getTestLogger("BlockIndex"));
    auto converter =
        std::make_shared<shared_model::proto::ProtoBlockJsonConverter>();
    blocks = std::make_shared<PostgresBlockQuery>(
        *sql, *file, converter, getTestLogger("BlockQuery"));
    empty_blocks = std::make_shared<PostgresBlockQuery>(
        *sql, *mock_file, converter, getTestLogger("PostgresBlockQueryEmpty"));

    *sql << init_;

    // First transaction in block1
    auto txn1_1 = TestTransactionBuilder().creatorAccountId(creator1).build();
    tx_hashes.push_back(txn1_1.hash());

    // Second transaction in block1
    auto txn1_2 = TestTransactionBuilder().creatorAccountId(creator1).build();
    tx_hashes.push_back(txn1_2.hash());

    std::vector<shared_model::proto::Transaction> txs1;
    txs1.push_back(std::move(txn1_1));
    txs1.push_back(std::move(txn1_2));

    auto block1 =
        TestBlockBuilder()
            .height(1)
            .transactions(txs1)
            .prevHash(shared_model::crypto::Hash(zero_string))
            .rejectedTransactions(
                std::vector<shared_model::crypto::Hash>{rejected_hash})
            .build();

    // First tx in block 1
    auto txn2_1 = TestTransactionBuilder().creatorAccountId(creator1).build();
    tx_hashes.push_back(txn2_1.hash());

    // Second tx in block 2
    auto txn2_2 = TestTransactionBuilder().creatorAccountId(creator2).build();
    tx_hashes.push_back(txn2_2.hash());

    std::vector<shared_model::proto::Transaction> txs2;
    txs2.push_back(std::move(txn2_1));
    txs2.push_back(std::move(txn2_2));

    auto block2 = TestBlockBuilder()
                      .height(2)
                      .transactions(txs2)
                      .prevHash(block1.hash())
                      .build();

    for (const auto &b : {std::move(block1), std::move(block2)}) {
      converter->serialize(b).match(
          [this, &b](const iroha::expected::Value<std::string> &json) {
            file->add(b.height(), iroha::stringToBytes(json.value));
            index->index(b);
            blocks_total++;
          },
          [](const auto &error) { FAIL() << error.error; });
    }
  }

  void TearDown() override {
    sql->close();
    AmetsuchiTest::TearDown();
  }

  std::unique_ptr<soci::session> sql;
  std::vector<shared_model::crypto::Hash> tx_hashes;
  std::shared_ptr<BlockQuery> blocks;
  std::shared_ptr<BlockQuery> empty_blocks;
  std::shared_ptr<BlockIndex> index;
  std::unique_ptr<FlatFile> file;
  std::shared_ptr<MockKeyValueStorage> mock_file;
  std::string creator1 = "user1@test";
  std::string creator2 = "user2@test";
  std::size_t blocks_total{0};
  std::string zero_string = std::string(32, '0');
  shared_model::crypto::Hash rejected_hash{"rejected_tx_hash"};
};

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get non-existent 1000th block
 * @then nothing is returned
 */
TEST_F(BlockQueryTest, GetNonExistentBlock) {
  auto stored_blocks = blocks->getBlocks(1000, 1);
  ASSERT_TRUE(stored_blocks.empty());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when height=1, count=1
 * @then returned exactly 1 block
 */
TEST_F(BlockQueryTest, GetExactlyOneBlock) {
  auto stored_blocks = blocks->getBlocks(1, 1);
  ASSERT_EQ(stored_blocks.size(), 1);
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when count=0
 * @then no blocks returned
 */
TEST_F(BlockQueryTest, GetBlocks_Count0) {
  auto stored_blocks = blocks->getBlocks(1, 0);
  ASSERT_TRUE(stored_blocks.empty());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get zero block
 * @then no blocks returned
 */
TEST_F(BlockQueryTest, GetZeroBlock) {
  auto stored_blocks = blocks->getBlocks(0, 1);
  ASSERT_TRUE(stored_blocks.empty());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get all blocks starting from 1
 * @then returned all blocks (2)
 */
TEST_F(BlockQueryTest, GetBlocksFrom1) {
  auto stored_blocks = blocks->getBlocksFrom(1);
  ASSERT_EQ(stored_blocks.size(), blocks_total);
  for (size_t i = 0; i < stored_blocks.size(); i++) {
    auto b = stored_blocks[i];
    ASSERT_EQ(b->height(), i + 1)
        << "block height: " << b->height() << "counter: " << i;
  }
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test. Block #1 is filled with trash data
 * (NOT JSON).
 * @when read block #1
 * @then get no blocks
 */
TEST_F(BlockQueryTest, GetBlockButItIsNotJSON) {
  namespace fs = boost::filesystem;
  size_t block_n = 1;

  // write something that is NOT JSON to block #1
  auto block_path = fs::path{block_store_path} / FlatFile::id_to_name(block_n);
  fs::ofstream block_file(block_path);
  std::string content = R"(this is definitely not json)";
  block_file << content;
  block_file.close();

  auto stored_blocks = blocks->getBlocks(block_n, 1);
  ASSERT_TRUE(stored_blocks.empty());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test. Block #1 is filled with trash data
 * (NOT JSON).
 * @when read block #1
 * @then get no blocks
 */
TEST_F(BlockQueryTest, GetBlockButItIsInvalidBlock) {
  namespace fs = boost::filesystem;
  size_t block_n = 1;

  // write bad block instead of block #1
  auto block_path = fs::path{block_store_path} / FlatFile::id_to_name(block_n);
  fs::ofstream block_file(block_path);
  std::string content = R"({
  "testcase": [],
  "description": "make sure this is valid json, but definitely not a block"
})";
  block_file << content;
  block_file.close();

  auto stored_blocks = blocks->getBlocks(block_n, 1);
  ASSERT_TRUE(stored_blocks.empty());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test AND 1 tx created by user2@test
 * @when get top 2 blocks
 * @then last 2 blocks returned with correct height
 */
TEST_F(BlockQueryTest, GetTop2Blocks) {
  size_t blocks_n = 2;  // top 2 blocks

  auto stored_blocks = blocks->getTopBlocks(blocks_n);
  ASSERT_EQ(stored_blocks.size(), blocks_n);

  for (size_t i = 0; i < blocks_n; i++) {
    auto b = stored_blocks[i];
    ASSERT_EQ(b->height(), i + 1);
  }
}

/**
 * @given block store with preinserted blocks
 * @when checkTxPresence is invoked on existing transaction hash
 * @then Committed status is returned
 */
TEST_F(BlockQueryTest, HasTxWithExistingHash) {
  for (const auto &hash : tx_hashes) {
    ASSERT_NO_THROW({
      auto status = boost::get<tx_cache_status_responses::Committed>(
          *blocks->checkTxPresence(hash));
      ASSERT_EQ(status.hash, hash);
    });
  }
}

/**
 * @given block store with preinserted blocks
 * user1@test AND 1 tx created by user2@test
 * @when checkTxPresence is invoked on non-existing hash
 * @then Missing status is returned
 */
TEST_F(BlockQueryTest, HasTxWithMissingHash) {
  shared_model::crypto::Hash missing_tx_hash(zero_string);
  ASSERT_NO_THROW({
    auto status = boost::get<tx_cache_status_responses::Missing>(
        *blocks->checkTxPresence(missing_tx_hash));
    ASSERT_EQ(status.hash, missing_tx_hash);
  });
}

/**
 * @given block store with preinserted blocks containing rejected_hash1 in one
 * of the block
 * @when checkTxPresence is invoked on existing rejected hash
 * @then Rejected is returned
 */
TEST_F(BlockQueryTest, HasTxWithRejectedHash) {
  ASSERT_NO_THROW({
    auto status = boost::get<tx_cache_status_responses::Rejected>(
        *blocks->checkTxPresence(rejected_hash));
    ASSERT_EQ(status.hash, rejected_hash);
  });
}

/**
 * @given block store with preinserted blocks
 * @when getTopBlock is invoked on this block store
 * @then returned top block's height is equal to the inserted one's
 */
TEST_F(BlockQueryTest, GetTopBlockSuccess) {
  auto top_block_opt = framework::expected::val(blocks->getTopBlock());
  ASSERT_TRUE(top_block_opt);
  ASSERT_EQ(top_block_opt.value().value->height(), 2);
}

/**
 * @given empty block store
 * @when getTopBlock is invoked on this block store
 * @then result must be a string error, because no block was fetched
 */
TEST_F(BlockQueryTest, GetTopBlockFail) {
  EXPECT_CALL(*mock_file, last_id()).WillRepeatedly(Return(0));
  EXPECT_CALL(*mock_file, get(mock_file->last_id()))
      .WillOnce(Return(boost::none));

  auto top_block_error = framework::expected::err(empty_blocks->getTopBlock());
  ASSERT_TRUE(top_block_error);
  auto expected_error = boost::format("Failed to retrieve block with id %d");
  ASSERT_EQ(top_block_error.value().error,
            (expected_error % mock_file->last_id()).str());
}
