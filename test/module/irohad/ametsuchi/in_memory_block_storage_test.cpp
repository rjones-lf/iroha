/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/in_memory_block_storage.hpp"
#include "ametsuchi/impl/in_memory_block_storage_factory.hpp"

#include <gtest/gtest.h>
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::ametsuchi;
using ::testing::Invoke;
using ::testing::NiceMock;

class InMemoryBlockStorageTest : public ::testing::Test {
 public:
  InMemoryBlockStorageTest() {
    ON_CALL(*block_, clone()).WillByDefault(Invoke([this] {
      block_copy_ = new NiceMock<MockBlock>();
      return block_copy_;
    }));
  }

 protected:
  void TearDown() override {
    block_storage_.clear();
  }

  InMemoryBlockStorage block_storage_;
  std::shared_ptr<MockBlock> block_ = std::make_shared<NiceMock<MockBlock>>();
  MockBlock *block_copy_;
  InMemoryBlockStorage::Identifier id_ = 1;
};

/**
 * @given block storage factory
 * @when create is called
 * @then block storage is created
 */
TEST(InMemoryBlockStorageFactoryTest, Creation) {
  InMemoryBlockStorageFactory factory;

  ASSERT_TRUE(factory.create());
}

/**
 * @given initialized block storage, single block with id_ inserted
 * @when another block with id_ is inserted
 * @then second insertion fails
 */
TEST_F(InMemoryBlockStorageTest, Insert) {
  ASSERT_TRUE(block_storage_.insert(id_, *block_));

  ASSERT_FALSE(block_storage_.insert(id_, *block_));
}

/**
 * @given initialized block storage, single block with id_ inserted
 * @when block with id_ is fetched
 * @then it is returned
 */
TEST_F(InMemoryBlockStorageTest, FetchExisting) {
  ASSERT_TRUE(block_storage_.insert(id_, *block_));

  auto block_var = block_storage_.fetch(id_);

  ASSERT_EQ(block_copy_, block_var->get());
}

/**
 * @given initialized block storage without blocks
 * @when block with id_ is fetched
 * @then nothing is returned
 */
TEST_F(InMemoryBlockStorageTest, FetchNonexistent) {
  auto block_var = block_storage_.fetch(id_);

  ASSERT_FALSE(block_var);
}

/**
 * @given initialized block storage, single block with id_ inserted
 * @when size is fetched
 * @then 1 is returned
 */
TEST_F(InMemoryBlockStorageTest, Size) {
  ASSERT_TRUE(block_storage_.insert(id_, *block_));

  ASSERT_EQ(1, block_storage_.size());
}

/**
 * @given initialized block storage, single block with id_ inserted
 * @when storage is cleared with clear
 * @then no blocks are left in storage
 */
TEST_F(InMemoryBlockStorageTest, Clear) {
  ASSERT_TRUE(block_storage_.insert(id_, *block_));

  block_storage_.clear();

  auto block_var = block_storage_.fetch(id_);

  ASSERT_FALSE(block_var);
}

/**
 * @given initialized block storage, single block with id_ inserted
 * @when forEach is called
 * @then block with id_ is visited, lambda is invoked once
 */
TEST_F(InMemoryBlockStorageTest, ForEach) {
  ASSERT_TRUE(block_storage_.insert(id_, *block_));

  size_t count = 0;

  block_storage_.forEach([this, &count](auto id, const auto &block) {
    ++count;
    ASSERT_EQ(id_, id);
    ASSERT_EQ(block_copy_, block.get());
  });

  ASSERT_EQ(1, count);
}
