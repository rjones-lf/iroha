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

#ifndef IROHA_AMETSUCHI_FIXTURE_HPP
#define IROHA_AMETSUCHI_FIXTURE_HPP

#include "common/files.hpp"
#include "logger/logger.hpp"

#include <gtest/gtest.h>
#include <cpp_redis/cpp_redis>
#include <pqxx/pqxx>

#include "model/generators/command_generator.hpp"

namespace iroha {
  namespace ametsuchi {
    /**
     * Class with ametsuchi initialization
     */
    class AmetsuchiTest : public ::testing::Test {
     public:
      AmetsuchiTest() {
        auto log = logger::testLog("AmetsuchiTest");

        mkdir(block_store_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        auto pg_host = std::getenv("IROHA_POSTGRES_HOST");
        auto pg_port = std::getenv("IROHA_POSTGRES_PORT");
        auto pg_user = std::getenv("IROHA_POSTGRES_USER");
        auto pg_pass = std::getenv("IROHA_POSTGRES_PASSWORD");
        auto rd_host = std::getenv("IROHA_REDIS_HOST");
        auto rd_port = std::getenv("IROHA_REDIS_PORT");
        if (not pg_host) {
          return;
        }
        std::stringstream ss;
        ss << "host=" << pg_host << " port=" << pg_port << " user=" << pg_user
           << " password=" << pg_pass;
        pgopt_ = ss.str();
        redishost_ = rd_host;
        redisport_ = std::stoull(rd_port);
        log->info("host={}, port={}, user={}, password={}", pg_host, pg_port,
                  pg_user, pg_pass);
      }
     protected:
      virtual void SetUp() {
        connection = std::make_shared<pqxx::lazyconnection>(pgopt_);
        try {
          connection->activate();
        } catch (const pqxx::broken_connection &e) {
          FAIL() << "Connection to PostgreSQL broken: " << e.what();
        }
        try {
          client.connect(redishost_, redisport_);
        } catch (const cpp_redis::redis_error &e) {
          FAIL() << "Connection to Redis broken: " << e.what();
        }
      }
      virtual void TearDown() {
        const auto drop = R"(
DROP TABLE IF EXISTS account_has_signatory;
DROP TABLE IF EXISTS account_has_asset;
DROP TABLE IF EXISTS role_has_permissions;
DROP TABLE IF EXISTS account_has_roles;
DROP TABLE IF EXISTS account_has_grantable_permissions;
DROP TABLE IF EXISTS account;
DROP TABLE IF EXISTS asset;
DROP TABLE IF EXISTS domain;
DROP TABLE IF EXISTS signatory;
DROP TABLE IF EXISTS peer;
DROP TABLE IF EXISTS role;
)";

        pqxx::work txn(*connection);
        txn.exec(drop);
        txn.commit();
        connection->disconnect();

        client.flushall();
        client.sync_commit();
        client.disconnect(true);

        iroha::remove_all(block_store_path);
      }

      std::shared_ptr<pqxx::lazyconnection> connection;

      cpp_redis::client client;

      model::generators::CommandGenerator cmd_gen;

      std::string pgopt_ =
          "host=localhost port=5432 user=postgres password=mysecretpassword";

      std::string redishost_ = "localhost";
      size_t redisport_ = 6379;

      std::string block_store_path = "/tmp/block_store";
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_FIXTURE_HPP
