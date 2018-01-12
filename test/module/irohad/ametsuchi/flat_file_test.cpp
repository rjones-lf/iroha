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

#include <sys/stat.h>
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/impl/flat_file/flat_file.cpp"

#include <gtest/gtest.h>
#include "common/files.hpp"
#include "common/types.hpp"

#include "logger/logger.hpp"

using namespace iroha::ametsuchi;

static logger::Logger log_ = logger::testLog("BlockStore");

class BlStore_Test : public ::testing::Test {
 protected:
  void SetUp() override {
    mkdir(block_store_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  void TearDown() override {
    iroha::remove_all(block_store_path);
    rmdir(block_store_path.c_str());
  }
  std::string block_store_path = "/tmp/dump";
};

TEST_F(BlStore_Test, Read_Write_Test) {
  std::vector<uint8_t> block(100000, 5);
  auto bl_store = FlatFile::create(block_store_path);
  ASSERT_TRUE(bl_store);

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
    log_->info("----------| create blockstore and insert 3 elements "
                   "|----------");

    auto bl_store = FlatFile::create(block_store_path);
    ASSERT_TRUE(bl_store);

    std::vector<uint8_t> block(1000, 5);
    // Adding three blocks
    auto id = 1u;
    bl_store->add(id, block);
    auto id2 = 2u;
    bl_store->add(id2, block);
    auto id3 = 3u;
    bl_store->add(id3, block);
  }

  log_->info("----------| remove second and init new storage |----------");
  std::remove((block_store_path + "/0000000000000002").c_str());
  std::vector<uint8_t> block(1000, 5);
  auto bl_store = FlatFile::create(block_store_path);
  ASSERT_TRUE(bl_store);
  auto res = bl_store->last_id();
  ASSERT_EQ(res, 1);
}

TEST_F(BlStore_Test, BlockStoreWhenAbsentFolder) {
  log_->info("----------| Check that folder absent => create => "
                 "make storage => remove storage |----------");
  std::string target_path = "/tmp/bump";
  rmdir(target_path.c_str());
  {
    auto bl_store = FlatFile::create(block_store_path);
    ASSERT_TRUE(bl_store);
    std::vector<uint8_t> block(100000, 5);
    auto id = 1u;
    bl_store->add(id, block);
    auto res = bl_store->last_id();
    ASSERT_EQ(res, 1);
  }
  rmdir(target_path.c_str());
}

/**
 * @given non-empty folder from previous block store
 * @when new block storage is initialized
 * @then new block storage has all blocks from the folder
 */
TEST_F(BlStore_Test, BlockStoreInitializationFromNonemptyFolder){
  auto bl_store1 = FlatFile::create(block_store_path);
  ASSERT_TRUE(bl_store1);

  // Add two blocks to storage
  bl_store1->add(1u, std::vector<uint8_t>(1000, 5));
  bl_store1->add(2u, std::vector<uint8_t>(1000, 5));

  // create second block storage from the same folder
  auto bl_store2 = FlatFile::create(block_store_path);
  ASSERT_TRUE(bl_store2);

  // check that last ids of both block storages are the same
  ASSERT_EQ(bl_store1->last_id(), bl_store2->last_id());
}

TEST_F(BlStore_Test, EmptyDumpDir) {
  auto res = check_consistency("");
  ASSERT_EQ(res, nonstd::nullopt);
}

TEST_F(BlStore_Test, CreateFileEmptyDir) {
  //auto res = FlatFile::create("");
  //ASSERT_EQ(res, nullptr);
}


TEST_F(BlStore_Test, GetNonExistingFile) {
  auto bl_store = FlatFile::create(block_store_path);
  Identifier id = 98759385; //random number that will not exist
  auto res = bl_store->get(id);
  ASSERT_EQ(res, nonstd::nullopt);
}

TEST_F(BlStore_Test, GetDirectory) {
  auto bl_store = FlatFile::create(block_store_path);
  ASSERT_EQ(bl_store->directory(), block_store_path);
}

TEST_F(BlStore_Test, GetDeniedBlock) {
  std::vector<uint8_t> block(100000, 5);
  auto bl_store = FlatFile::create(block_store_path);
  auto id = 1u;
  bl_store->add(id, block);

  auto filename = boost::filesystem::path{block_store_path} / id_to_name(id);
  chmod(filename.string().data(), 0);

  auto res = bl_store->get(id);
  ASSERT_EQ(res, nonstd::nullopt);
}

TEST_F(BlStore_Test, AddExistingId) {
  std::vector<uint8_t> block(100000, 5);
  auto bl_store = FlatFile::create(block_store_path);
  auto id = 1u;
  const auto file_name = boost::filesystem::path{block_store_path} / id_to_name(id);
  std::ofstream fout(file_name.string());
  fout.close();

  auto res = bl_store->add(id, block);
  ASSERT_FALSE(res);
}

TEST_F(BlStore_Test, WriteDeniedFolder) {
  std::vector<uint8_t> block(100000, 5);
  auto bl_store = FlatFile::create(block_store_path);
  auto id = 1u;
  const auto file_name = boost::filesystem::path{block_store_path} / id_to_name(id);
  chmod(block_store_path.data(), 0500);
  auto res = bl_store->add(id, block);
  ASSERT_FALSE(res);
}