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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "ametsuchi/impl/flat_file/flat_file.hpp"

#include <gtest/gtest.h>
#include "common/files.hpp"
#include "common/types.hpp"

#include "logger/logger.hpp"

using namespace iroha::ametsuchi;
using Identifier = FlatFile::Identifier;

static logger::Logger log_ = logger::testLog("BlockStore");

class BlStore_Test : public ::testing::Test {
 protected:
  void clear() {
    try {
      namespace fs = boost::filesystem;
      using perms = boost::filesystem::perms;
      fs::permissions(
          block_store_path,
          perms::owner_read | perms::owner_write | perms::owner_exe);
      fs::remove_all(block_store_path);
    } catch (...) {
    }
  }

  void SetUp() override {
    block = std::vector<uint8_t>(16, 'a');
    clear();
  }

  void TearDown() override {
    clear();
  }

  std::string block_store_path = "/tmp/iroha_block_storage";
  std::vector<uint8_t> block;
};

TEST_F(BlStore_Test, Read_Write_Test) {
  size_t n = FlatFile::kFirstBlockAt;
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  bl_store->add(n++, block);
  bl_store->add(n++, block);

  auto res = bl_store->get(FlatFile::kFirstBlockAt);
  ASSERT_TRUE(res);
  ASSERT_FALSE(res->empty());

  ASSERT_EQ(res->size(), block.size());
  ASSERT_EQ(*res, block);
}

/**
 * @given non-empty folder from previous block store
 * @when new block storage is initialized
 * @then new block storage has all blocks from the folder
 */
TEST_F(BlStore_Test, BlockStoreInitializationFromNonemptyFolder) {
  size_t n = FlatFile::kFirstBlockAt;
  uint64_t total1 = 0u;
  {
    auto store = FlatFile::create(block_store_path);
    ASSERT_TRUE(store);
    auto bl_store1 = std::move(*store);

    // Add two blocks to storage
    ASSERT_TRUE(bl_store1->add(n++, block));
    ASSERT_TRUE(bl_store1->add(n++, block));

    total1 = bl_store1->last_id();
  }

  // create second block storage from the same folder
  auto store2 = FlatFile::create(block_store_path);
  ASSERT_TRUE(store2);
  auto bl_store2 = std::move(*store2);

  // check that last ids of both block storages are the same
  ASSERT_EQ(total1, bl_store2->last_id());
}

/**
 * @given empty folder with block store
 * @when block id does not exist
 * @then get() fails
 */
TEST_F(BlStore_Test, GetNonExistingFile) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  Identifier id = 98759385;  // random number that does not exist
  auto res = bl_store->get(id);
  ASSERT_FALSE(res);
}

/**
 * @given empty folder with block store
 * @when FlatFile was created
 * @then directory() returns bock store path
 */
TEST_F(BlStore_Test, GetDirectory) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  ASSERT_EQ(bl_store->directory(), block_store_path);
}

/**
 * @given empty folder
 * @when tries to create FlatFile with empty path
 * @then FlatFile creation fails
 */
TEST_F(BlStore_Test, WriteEmptyFolder) {
  auto bl_store = FlatFile::create("");
  ASSERT_FALSE(bl_store);
}

TEST_F(BlStore_Test, WriteThenReadSequential) {
  auto s = FlatFile::create(block_store_path);
  ASSERT_TRUE(s);
  auto bs = std::move(*s);

  for (size_t i = 0x00 + FlatFile::kFirstBlockAt; i < 0xff; i++) {
    auto v = std::vector<uint8_t>(16, i);
    auto total = bs->last_id();
    ASSERT_TRUE(bs->add(i, v)) << "can not add block";
    ASSERT_EQ(total + 1, bs->last_id());

    auto item = bs->get(i);
    if (not item) {
      FAIL() << "wrote item " << i << " then read empty";
    } else {
      ASSERT_EQ(*item, v);
    }
  }
}
