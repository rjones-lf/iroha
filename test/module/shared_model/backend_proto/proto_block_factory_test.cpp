/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "backend/protobuf/proto_block_factory.hpp"
#include "datetime/time.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/validators/validators.hpp"
#include "validators/default_validator.hpp"

using namespace shared_model;

class ProtoBlockFactoryTest : public ::testing::Test {
 public:
  std::unique_ptr<proto::ProtoBlockFactory> factory;
  validation::MockValidator<interface::Block> *validator;

  ProtoBlockFactoryTest() {
    auto validator_ptr =
        std::make_unique<validation::MockValidator<interface::Block>>();
    validator = validator_ptr.get();
    factory =
        std::make_unique<proto::ProtoBlockFactory>(std::move(validator_ptr));
  }
};

/**
 * @given valid data for block
 * @when block is created using unsafeCreateBlock function
 * @then block fields match provided data
 */
TEST_F(ProtoBlockFactoryTest, UnsafeBlockCreation) {
  int height = 1;
  auto created_time = iroha::time::now();
  auto prev_hash = shared_model::crypto::Hash::fromHexString("123456");

  std::vector<shared_model::proto::Transaction> txs;
  txs.emplace_back(iroha::protocol::Transaction{});

  auto block = factory->unsafeCreateBlock(height, prev_hash, created_time, txs);

  ASSERT_EQ(block->height(), height);
  ASSERT_EQ(block->createdTime(), created_time);
  ASSERT_EQ(block->prevHash().hex(), prev_hash.hex());
  ASSERT_EQ(block->transactions(), txs);
}
