/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"

using namespace iroha::ametsuchi;
using testing::Bool;
using testing::Values;
using testing::WithParamInterface;

class MutableStorageTest : public AmetsuchiTest,
                           public WithParamInterface<bool> {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();

    storage->createMutableStorage().match(
        [this](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                   &mut_storage) {
          mutable_storage_ = std::move(mut_storage.value);
        },
        [](const auto &) { FAIL() << "Mutable storage cannot be created"; });
  }

  void TearDown() override {
    mutable_storage_.reset();
    AmetsuchiTest::TearDown();
  };

  auto getBlock() {
    return TestBlockBuilder()
        .transactions(std::vector<shared_model::proto::Transaction>({}))
        .height(1)
        .prevHash(fake_hash)
        .build();
  }

  std::string zero_string{32, '0'};
  shared_model::crypto::Hash fake_hash{zero_string};
  shared_model::crypto::PublicKey fake_pubkey{zero_string};
  std::unique_ptr<MutableStorage> mutable_storage_;
};

/**
 * @given mutable storage
 * @when check block method takes block and predicated returning boolean value
 * @then the same block is processed in predicate
 * @and returned value returned by check function is the same as result of
 * predicate
 */
TEST_P(MutableStorageTest, TestCheckBlock) {
  auto expected_block = getBlock();
  bool expected_res = GetParam();
  ASSERT_EQ(expected_res,
            mutable_storage_->check(
                expected_block,
                [&expected_block, &expected_res](
                    const auto &block, const auto &, const auto &) {
                  EXPECT_EQ(expected_block, block);
                  return expected_res;
                }));
}

INSTANTIATE_TEST_CASE_P(MutableStorageParameterizedTest,
                        MutableStorageTest,
                        // note additional comma is needed to make it compile
                        // https://github.com/google/googletest/issues/1419
                        Bool(), );
