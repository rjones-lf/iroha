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
#include <rapidjson/document.h>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <pqxx/pqxx>

#include "common/files.hpp"
#include "main/iroha_conf_loader.hpp"

using namespace boost::process;
using namespace std::chrono_literals;

class IrohadTest : public testing::Test {
 public:
  virtual void SetUp() {
    setPaths();
    auto config = parse_iroha_config(path_config.string());
    blockstore_path = config[config_members::BlockStorePath].GetString();
    pgopts = config[config_members::PgOpt].GetString();
  }
  virtual void TearDown() {
    iroha::remove_all(blockstore_path);
    dropPostgres();
  }

 private:
  void setPaths() {
    path_irohad = boost::filesystem::path(PATHIROHAD);
    irohad_executable = path_irohad / "irohad";
    path_example = path_irohad / ".." / ".." / "example";
    path_config = path_example / "config.sample";
    path_genesis = path_example / "genesis.block";
    path_keypair = path_example / "node0";
    params = " --config " + path_config.string() + " --genesis_block "
        + path_genesis.string() + " --keypair_name " + path_keypair.string();
    timeout = 1s;
  }

  void dropPostgres() {
    connection = std::make_shared<pqxx::lazyconnection>(pgopts);
    try {
      connection->activate();
    } catch (const pqxx::broken_connection &e) {
      FAIL() << "Connection to PostgreSQL broken: " << e.what();
    }

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
DROP TABLE IF EXISTS height_by_hash;
DROP TABLE IF EXISTS height_by_account_set;
DROP TABLE IF EXISTS index_by_creator_height;
DROP TABLE IF EXISTS index_by_id_height_asset;
)";

    pqxx::work txn(*connection);
    txn.exec(drop);
    txn.commit();
    connection->disconnect();
  }

 public:
  boost::filesystem::path path_irohad;
  boost::filesystem::path irohad_executable;
  boost::filesystem::path path_example;
  boost::filesystem::path path_config;
  boost::filesystem::path path_genesis;
  boost::filesystem::path path_keypair;
  std::string params;
  std::chrono::milliseconds timeout;
  std::shared_ptr<pqxx::lazyconnection> connection;
  std::string pgopts;
  std::string blockstore_path;
};

/*
 * @given path to irohad executable
 * @when run irohad without any parameters
 * @then irohad should not start
 */
TEST_F(IrohadTest, RunIrohadWithoutArgs) {
  child c(irohad_executable);
  c.wait_for(timeout);
  ASSERT_FALSE(c.running());
}

/*
 * @given path to irohad executable and paths to files irohad is needed to be
 * run (config, genesis block, keypair)
 * @when run irohad with all parameters it needs to operate as a full node
 * @then irohad should be started and running until timeout expired
 */
TEST_F(IrohadTest, RunIrohad) {
  child c(irohad_executable.string() + params);
  std::this_thread::sleep_for(timeout);
  ASSERT_TRUE(c.running());
  c.terminate();
}
