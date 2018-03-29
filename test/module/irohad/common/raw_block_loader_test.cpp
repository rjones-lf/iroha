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

#include <gtest/gtest.h>

#include "interfaces/iroha_internal/block.hpp"
#include "main/raw_block_loader.hpp"

using iroha::main::BlockLoader;

/**
 * @given block in json format
 * @when converting json to block using raw block loader
 * @then check that the block is correct
 */
TEST(BlockLoaderTest, BlockLoaderJsonParsing) {
  BlockLoader loader;
  auto str =
      R"({
"payload": {
  "transactions": [],
  "height": 1,
  "prev_block_hash": "AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQE=",
  "created_time": 0
  },
"signatures": []
})";

  auto block = loader.parseBlock(str);

  ASSERT_TRUE(block);
  auto b = block.value();

  ASSERT_EQ(b->transactions().size(), 0);
  ASSERT_EQ(b->height(), 1);
  ASSERT_EQ(b->createdTime(), 0);
  ASSERT_TRUE(b->signatures().empty());
  ASSERT_EQ(b->prevHash().hex(), "0101010101010101010101010101010101010101010101010101010101010101");
}
