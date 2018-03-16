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

#include <gtest/gtest.h>
#include "model/converters/json_block_factory.hpp"

using namespace iroha;
using namespace iroha::model;
using namespace iroha::model::converters;

class JsonBlockTest : public ::testing::Test {
 public:
  JsonBlockFactory factory;
};

TEST_F(JsonBlockTest, ValidWhenWellFormed) {
  Block orig_block{};

  auto json_block = factory.serialize(orig_block);
  auto serial_block = factory.deserialize(json_block);

  ASSERT_EQ(orig_block, serial_block);
}

TEST_F(JsonBlockTest, InvalidWhenFieldsMissing) {
  Block orig_block{};

  auto json_block = factory.serialize(orig_block);

  json_block.RemoveMember("created_ts");

  auto serial_block = factory.deserialize(json_block);

  ASSERT_FALSE(serial_block);
}
