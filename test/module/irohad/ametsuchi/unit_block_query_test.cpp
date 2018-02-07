/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "block_query_fixture.hpp"
#include "block_storage_mock.hpp"
#include "framework/test_subscriber.hpp"

using namespace framework::test_subscriber;
using namespace iroha::ametsuchi;
using namespace iroha::model;

class BlockQueryWithMock : public BlockQueryFixture {
 public:
  void SetUp() override {
    BlockQueryFixture::SetUp();

    bsmock_ = std::make_unique<BlockStorageMock>();
    ASSERT_TRUE(bsmock_) << "block storage failed";

    index = std::make_shared<RedisBlockIndex>(client);
    query = std::make_shared<RedisBlockQuery>(client, *bsmock_);
  }

  std::unique_ptr<BlockStorageMock> bsmock_;
};

/////////////////////////////////////////////////////////
/// BlockQueryWithMock

/**
 * @given block store with few blocks, with block #1 = garbage
 * @when read block #1
 * @then get no blocks
 */
TEST_F(BlockQueryWithMock, GetBlockButItIsNotJSON) {
  namespace fs = boost::filesystem;
  using ::testing::AtLeast;
  using ::testing::Eq;
  using ::testing::Exactly;
  using ::testing::Return;

  BlockStorage::Identifier block_n = 1;

  const std::string s("something that is not json");
  const std::vector<uint8_t> garbage{s.begin(), s.end()};

  // get(block_n) is called once to fetch the block
  EXPECT_CALL(*bsmock_, get(Eq(block_n)))
      .Times(Exactly(1))
      .WillRepeatedly(Return(garbage));

  // total number of blocks in block storage
  EXPECT_CALL(*bsmock_, total_keys())
      .Times(AtLeast(1))
      .WillRepeatedly(Return(blocks.size()));

  auto wrapper =
      make_test_subscriber<CallExact>(query->getBlocks(block_n, 1), 0);

  wrapper.subscribe();

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with few blocks, where block #1 is not json
 * @when read block #1
 * @then get no blocks
 */
TEST_F(BlockQueryWithMock, GetBlockButItIsInvalidBlock) {
  namespace fs = boost::filesystem;
  using ::testing::AtLeast;
  using ::testing::Eq;
  using ::testing::Exactly;
  using ::testing::Return;

  BlockStorage::Identifier block_n = 1;

  const std::string s = R"({
    "testcase": [],
    "description": "make sure this is valid json, but definitely not a block"
  })";
  const std::vector<uint8_t> notjson{s.begin(), s.end()};

  // get(block_n) is called once to fetch the block and it returns notjson
  EXPECT_CALL(*bsmock_, get(Eq(block_n)))
      .Times(Exactly(1))
      .WillRepeatedly(Return(notjson));

  // whenever total_keys() is called, it returns 3 blocks
  EXPECT_CALL(*bsmock_, total_keys())
      .Times(AtLeast(1))
      .WillRepeatedly(Return(blocks.size()));

  auto wrapper =
      make_test_subscriber<CallExact>(query->getBlocks(block_n, 1), 0);
  wrapper.subscribe();

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks
 * @when read top 2 blocks
 * @then get blocks
 */
TEST_F(BlockQueryWithMock, GetTopAllBlocks) {
  namespace fs = boost::filesystem;
  using ::testing::AtLeast;
  using ::testing::Eq;
  using ::testing::Exactly;
  using ::testing::Return;


  for (size_t i = 0; i < blocks.size(); i++) {
    std::vector<uint8_t> expected = iroha::stringToBytes(
        converters::jsonToString(conv->serialize(blocks[i])));

    // get(block_n) is called once to fetch the block and it returns block
    // #block_n
    EXPECT_CALL(*bsmock_, get(Eq(i)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(expected));
  }

  EXPECT_CALL(*bsmock_, total_keys())
      .Times(AtLeast(1))
      .WillRepeatedly(Return(blocks.size()));

  auto wrapper = make_test_subscriber<CallExact>(
      query->getTopBlocks(static_cast<uint32_t>(blocks.size())), blocks.size());

  int block_n = 0;

  // check that we received the same content as we wrote
  wrapper.subscribe([&](Block b) {
    std::vector<uint8_t> expected = iroha::stringToBytes(
        converters::jsonToString(conv->serialize(blocks[block_n])));

    std::vector<uint8_t> actual =
        iroha::stringToBytes(converters::jsonToString(conv->serialize(b)));

    ASSERT_EQ(expected, actual) << block_n;

    block_n++;
  });

  ASSERT_TRUE(wrapper.validate());
}
