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

#include "main/raw_block_loader.hpp"
#include <gtest/gtest.h>

#include "framework/test_block_generator.hpp"
#include "model/converters/json_block_factory.hpp"
#include "model/converters/json_common.hpp"
#include "model/converters/pb_command_factory.hpp"

using namespace iroha::main;
using namespace iroha::model::converters;
using namespace iroha;

/**
 * @given generated block
 *
 * @when convert block to JSON
 * AND parseBlock() with BlockLoader API
 *
 * @then check that blocks are equal
 */
TEST(BlockLoaderTest, BlockLoaderWhenParseBlock) {
  BlockLoader loader;
  auto block = framework::generator::generateBlock();
  auto doc = JsonBlockFactory().serialize(block);
  auto str = jsonToString(doc);
  auto new_block = loader.parseBlock(str);
  ASSERT_TRUE(new_block);
  ASSERT_EQ(block, *new_block);
}
