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
#include <ametsuchi/index/backend/redis.hpp>

namespace iroha {
  namespace ametsuchi {
    namespace index {

      class BlockIndex_Test : public ::testing::Test {
       protected:
        virtual void TearDown() {
          cpp_redis::redis_client client;
          client.connect(host_, port_);
          client.flushall();
          client.sync_commit();
          client.disconnect();
        }

        std::string host_ = "localhost";
        size_t port_ = 6379;
      };

      TEST_F(BlockIndex_Test, REDIS_ADD_GET_TEST_MULTI) {
        Redis block_index(host_, port_);

        ASSERT_TRUE(block_index.add_blockhash_blockid("one", 1));

        ASSERT_TRUE(block_index.exec_multi());

        auto one = block_index.get_blockid_by_blockhash("one");
        ASSERT_TRUE(one);
        ASSERT_EQ(*one, 1);
      }

      TEST_F(BlockIndex_Test, REDIS_ADD_GET_TEST_DISCARD) {
        Redis block_index(host_, port_);

        ASSERT_TRUE(block_index.add_blockhash_blockid("one", 1));

        ASSERT_TRUE(block_index.discard_multi());

        auto one = block_index.get_blockid_by_blockhash("one");
        ASSERT_FALSE(one);
      }

      TEST_F(BlockIndex_Test, REDIS_ADD_GET_TEST_READ) {
        Redis block_index(host_, port_);

        ASSERT_TRUE(block_index.add_blockhash_blockid("two", 2));

        ASSERT_TRUE(block_index.exec_multi());

        ASSERT_TRUE(block_index.add_blockhash_blockid("one", 1));

        auto two = block_index.get_blockid_by_blockhash("two");
        ASSERT_TRUE(two);
        ASSERT_EQ(*two, 2);

        ASSERT_TRUE(block_index.exec_multi());

        auto one = block_index.get_blockid_by_blockhash("one");
        ASSERT_TRUE(one);
        ASSERT_EQ(*one, 1);
      }

      TEST_F(BlockIndex_Test, REDIS_ADD_GET_PUBKEY_TXHASHES) {
        Redis block_index(host_, port_);

        ASSERT_TRUE(block_index.add_pubkey_txhash("1", "0"));
        ASSERT_TRUE(block_index.add_pubkey_txhash("1", "1"));
        ASSERT_TRUE(block_index.add_pubkey_txhash("1", "2"));
        ASSERT_TRUE(block_index.exec_multi());
        auto res = block_index.get_txhashes_by_pubkey("1");
        std::vector<std::string> expected({"0", "1", "2"});

        ASSERT_EQ(*res, expected);
      }
    }  // namespace index
  }    // namespace ametsuchi
}  // namespace iroha