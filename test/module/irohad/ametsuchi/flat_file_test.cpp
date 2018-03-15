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

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "common/files.hpp"
#include "common/types.hpp"

#include "logger/logger.hpp"

using namespace iroha::ametsuchi;
namespace fs = boost::filesystem;
using Identifier = FlatFile::Identifier;

static logger::Logger log_ = logger::testLog("BlockStore");

class BlStore_Test : public ::testing::Test {
 protected:
  void SetUp() override {
    fs::create_directory(block_store_path);
    block = std::vector<uint8_t>(100000, 5);
  }
  void TearDown() override {
    fs::remove_all(block_store_path);
  }
  std::string block_store_path =
      (fs::temp_directory_path() / fs::unique_path()).string();
  std::vector<uint8_t> block;
};

TEST_F(BlStore_Test, Read_Write_Test) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto id = 1u;

  bl_store->add(id, block);
  auto id2 = 2u;
  bl_store->add(id2, block);

  auto res = bl_store->get(id);
  ASSERT_TRUE(res);
  ASSERT_FALSE(res->empty());

  ASSERT_EQ(res->size(), block.size());
  ASSERT_EQ(*res, block);
}

TEST_F(BlStore_Test, BlockStoreWhenRemoveBlock) {
  log_->info("----------| Simulate removal of the block |----------");
  // Remove file in the middle of the block store
  {
    log_->info(
        "----------| create blockstore and insert 3 elements "
        "|----------");

    auto store = FlatFile::create(block_store_path);
    ASSERT_TRUE(store);
    auto bl_store = std::move(*store);

    // Adding three blocks
    auto id = 1u;
    bl_store->add(id, block);
    auto id2 = 2u;
    bl_store->add(id2, block);
    auto id3 = 3u;
    bl_store->add(id3, block);
  }

  log_->info("----------| remove second and init new storage |----------");
  fs::remove(fs::path(block_store_path) / "0000000000000002");
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto res = bl_store->last_id();
  ASSERT_EQ(res, 1);
}

TEST_F(BlStore_Test, BlockStoreWhenAbsentFolder) {
  log_->info(
      "----------| Check that folder is absent => create => "
      "make storage => remove storage |----------");
  fs::remove_all(block_store_path);
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto id = 1u;
  bl_store->add(id, block);
  auto res = bl_store->last_id();
  ASSERT_EQ(res, 1);
  fs::remove_all(block_store_path);
}

/**
 * @given non-empty folder from previous block store
 * @when new block storage is initialized
 * @then new block storage has all blocks from the folder
 */
TEST_F(BlStore_Test, BlockStoreInitializationFromNonemptyFolder) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store1 = std::move(*store);

  // Add two blocks to storage
  bl_store1->add(1u, std::vector<uint8_t>(1000, 5));
  bl_store1->add(2u, std::vector<uint8_t>(1000, 5));

  // create second block storage from the same folder
  auto store2 = FlatFile::create(block_store_path);
  ASSERT_TRUE(store2);
  auto bl_store2 = std::move(*store2);

  // check that last ids of both block storages are the same
  ASSERT_EQ(bl_store1->last_id(), bl_store2->last_id());
}

/**
 * @given empty folder name
 * @then check consistency fails
 */
TEST_F(BlStore_Test, EmptyDumpDir) {
  auto res = FlatFile::check_consistency("");
  ASSERT_FALSE(res);
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
 * @given block store with one entry
 * @when user has not enough permissions
 * @then get() fails
 */
TEST_F(BlStore_Test, GetDeniedBlock) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto id = 1u;
  bl_store->add(id, block);

  auto filename = fs::path(block_store_path) / FlatFile::id_to_name(id);

  fs::remove(filename);
  auto res = bl_store->get(id);
  ASSERT_FALSE(res);
}

/**
 * @given empty folder with one entry
 * @when tries to add an entry with an existing id
 * @then add() fails
 */
TEST_F(BlStore_Test, AddExistingId) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto id = 1u;
  const auto file_name = fs::path(block_store_path) / FlatFile::id_to_name(id);
  std::ofstream fout(file_name.string());
  fout.close();

  auto res = bl_store->add(id, block);
  ASSERT_FALSE(res);
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

/**
 * @given empty folder with block store
 * @when tries do add an entry having not enough permissions
 * @then add() fails
 */
TEST_F(BlStore_Test, WriteDeniedFolder) {
  auto store = FlatFile::create(block_store_path);
  ASSERT_TRUE(store);
  auto bl_store = std::move(*store);
  auto id = 1u;

  fs::remove(fs::path(block_store_path));
  auto res = bl_store->add(id, block);
  ASSERT_FALSE(res);
}
