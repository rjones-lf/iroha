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
#include <ametsuchi/wsv/backend/postgresql.hpp>
#include <pqxx/pqxx>
namespace iroha {
  namespace ametsuchi {
    namespace wsv {
      class WSVTest : public ::testing::Test {
       protected:
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

          pqxx::connection connection("host=" + host_ + " port=" +
                                      std::to_string(port_) + " user=" + user_ +
                                      " password=" + password_);
          pqxx::work txn(connection);
          txn.exec(drop);
          txn.commit();
          connection.disconnect();
        }
        std::string host_ = "localhost";
        size_t port_ = 5432;
        std::string user_ = "postgres";
        std::string password_ = "";
      };

      TEST_F(WSVTest, postgres_test) {
        auto wsv_ =
            std::make_unique<PostgreSQL>(host_, port_, user_, password_);

        std::string public_key("00000000000000000000000000000000");
        ASSERT_TRUE(wsv_->add_account(public_key, 1, 1));
        ASSERT_TRUE(wsv_->add_signatory(public_key, public_key));
        std::string address("127.0.0.1");
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));
        wsv_->commit_transaction();
        wsv_->commit_block();
        auto result = wsv_->get_peers(true);
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result.at(0), address);
      }

      TEST_F(WSVTest, select_test) {
        auto wsv_ =
            std::make_unique<PostgreSQL>(host_, port_, user_, password_);

        std::string public_key("00000000000000000000000000000000");
        ASSERT_TRUE(wsv_->add_account(public_key, 1, 1));
        std::string address("127.0.0.1");
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        wsv_->commit_transaction();

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        address = "127.0.0.2";
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 2);

        wsv_->commit_transaction();
        wsv_->commit_block();

        ASSERT_EQ(wsv_->get_peers(true).size(), 2);
      }

      TEST_F(WSVTest, rollback_test) {
        auto wsv_ =
            std::make_unique<PostgreSQL>(host_, port_, user_, password_);

        std::string public_key("00000000000000000000000000000000");
        ASSERT_TRUE(wsv_->add_account(public_key, 1, 1));

        wsv_->commit_transaction();

        std::string address("127.0.0.1");
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        wsv_->rollback_transaction();

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 0);

        address = "127.0.0.2";
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        wsv_->commit_transaction();
        wsv_->commit_block();

        ASSERT_EQ(wsv_->get_peers(true).size(), 1);
      }

      TEST_F(WSVTest, block_rollback_test) {
        auto wsv_ =
            std::make_unique<PostgreSQL>(host_, port_, user_, password_);

        std::string public_key("00000000000000000000000000000000");
        ASSERT_TRUE(wsv_->add_account(public_key, 1, 1));
        std::string address("127.0.0.1");
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        wsv_->commit_transaction();

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 1);

        address = "127.0.0.2";
        ASSERT_TRUE(wsv_->add_peer(public_key, address, 1));

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
        ASSERT_EQ(wsv_->get_peers(false).size(), 2);

        wsv_->commit_transaction();
        wsv_->rollback_block();

        ASSERT_EQ(wsv_->get_peers(true).size(), 0);
      }
    }  // namespace wsv

  }  // namespace ametsuchi
}  // namespace iroha