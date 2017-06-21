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

      class TxIndex_Test : public ::testing::Test {
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

      TEST_F(TxIndex_Test, MultiTest) {
        Redis tx_index(host_, port_);

        ASSERT_TRUE(tx_index.add_txhash_blockid_txid("tx_one", 1, 10));

        ASSERT_TRUE(tx_index.exec_multi());

        auto one = tx_index.get_blockid_by_txhash("tx_one");
        auto ten = tx_index.get_txid_by_txhash("tx_one");

        ASSERT_TRUE(one);
        ASSERT_TRUE(ten);

        ASSERT_EQ(*one, 1);
        ASSERT_EQ(*ten, 10);
      }

      TEST_F(TxIndex_Test, MultiTest_Discard) {
        Redis tx_index(host_, port_);

        ASSERT_TRUE(tx_index.add_txhash_blockid_txid("tx_one", 1, 10));

        ASSERT_TRUE(tx_index.discard_multi());

        auto one = tx_index.get_blockid_by_txhash("tx_one");
        auto ten = tx_index.get_txid_by_txhash("tx_one");

        ASSERT_FALSE(one);
        ASSERT_FALSE(ten);
      }

      TEST_F(TxIndex_Test, MultiTest_Read) {
        Redis tx_index(host_, port_);

        tx_index.add_txhash_blockid_txid("tx_two", 2, 20);

        ASSERT_TRUE(tx_index.exec_multi());

        auto two = tx_index.get_blockid_by_txhash("tx_two");
        auto twenty = tx_index.get_txid_by_txhash("tx_two");

        ASSERT_TRUE(two);
        ASSERT_TRUE(twenty);

        ASSERT_EQ(*two, 2);
        ASSERT_EQ(*twenty, 20);

        ASSERT_TRUE(tx_index.add_txhash_blockid_txid("tx_one", 1, 10));

        ASSERT_TRUE(tx_index.exec_multi());

        auto one = tx_index.get_blockid_by_txhash("tx_one");
        auto ten = tx_index.get_txid_by_txhash("tx_one");

        ASSERT_TRUE(one);
        ASSERT_TRUE(ten);

        ASSERT_EQ(*one, 1);
        ASSERT_EQ(*ten, 10);
      }

    }  // namespace index
  }    // namespace ametsuchi
}  // namespace iroha