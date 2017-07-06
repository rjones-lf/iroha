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
#include <ametsuchi/storage.hpp>
#include <common/types.hpp>
#include <cpp_redis/cpp_redis>
#include <pqxx/pqxx>

namespace iroha {
  namespace ametsuchi {

    class AmetsuchiTest : public ::testing::Test {
     protected:
      virtual void SetUp() {
        mkdir(block_store_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
      virtual void TearDown() {
        const auto drop =
            "DROP TABLE IF EXISTS domain_has_account;\n"
            "DROP TABLE IF EXISTS account_has_asset;\n"
            "DROP TABLE IF EXISTS account_has_wallet;\n"
            "DROP TABLE IF EXISTS wallet;\n"
            "DROP TABLE IF EXISTS exchange;\n"
            "DROP TABLE IF EXISTS asset;\n"
            "DROP TABLE IF EXISTS domain;\n"
            "DROP TABLE IF EXISTS peer;\n"
            "DROP TABLE IF EXISTS signatory;\n"
            "DROP TABLE IF EXISTS account;\n"
            "DROP SEQUENCE IF EXISTS peer_peer_id_seq;";

        pqxx::connection connection("host=" + pghost_ + " port=" +
                                    std::to_string(pgport_) + " user=" + user_ +
                                    " password=" + password_);
        pqxx::work txn(connection);
        txn.exec(drop);
        txn.commit();
        connection.disconnect();

        cpp_redis::redis_client client;
        client.connect(redishost_, redisport_);
        client.flushall();
        client.sync_commit();
        client.disconnect();

        remove_all(block_store_path);
      }

      std::string pghost_ = "localhost";
      size_t pgport_ = 5432;
      std::string user_ = "postgres";
      std::string password_ = "";

      std::string redishost_ = "localhost";
      size_t redisport_ = 6379;

      std::string block_store_path = "/tmp/block_store";
    };

    TEST_F(AmetsuchiTest, SampleTest) {
      auto ametsuchi_ = Storage::create();
      auto blob = std::vector<uint8_t>{0, 1, 2};
      ametsuchi_->insert_block(1, blob);
      auto block = ametsuchi_->get_block(1);
      ASSERT_EQ(block, blob);
    }

  }  // namespace ametsuchi
}  // namespace iroha