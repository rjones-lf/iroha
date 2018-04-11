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

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <pqxx/pqxx>
#include "ametsuchi/impl/storage_impl.hpp"
#include "common/files.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    /**
     * Class with ametsuchi initialization
     */
    class AmetsuchiTest : public ::testing::Test {
     public:
      AmetsuchiTest() {
        auto log = logger::testLog("AmetsuchiTest");

        boost::filesystem::create_directory(block_store_path);
        auto pg_host = std::getenv("IROHA_POSTGRES_HOST");
        auto pg_port = std::getenv("IROHA_POSTGRES_PORT");
        auto pg_user = std::getenv("IROHA_POSTGRES_USER");
        auto pg_pass = std::getenv("IROHA_POSTGRES_PASSWORD");
        if (not pg_host) {
          return;
        }
        std::stringstream ss;
        ss << "host=" << pg_host << " port=" << pg_port << " user=" << pg_user
           << " password=" << pg_pass;
        pgopt_ = ss.str();
        log->info("host={}, port={}, user={}, password={}",
                  pg_host,
                  pg_port,
                  pg_user,
                  pg_pass);
      }

     protected:
      virtual void clear() {
        pqxx::work txn(*connection);
        txn.exec(drop_);
        txn.commit();

        iroha::remove_dir_contents(block_store_path);
      }

      virtual void disconnect() {
        connection->disconnect();
      }

      virtual void connect() {
        connection = std::make_shared<pqxx::lazyconnection>(pgopt_);
        try {
          connection->activate();
        } catch (const pqxx::broken_connection &e) {
          FAIL() << "Connection to PostgreSQL broken: " << e.what();
        }

        StorageImpl::create(block_store_path, pgopt_)
            .match([&](iroha::expected::Value<std::shared_ptr<StorageImpl>>
                           &_storage) { storage = _storage.value; },
                   [](iroha::expected::Error<std::string> &error) {
                     FAIL() << "StorageImpl: " << error.error;
                   });
      }

      void SetUp() override {
        connect();
        storage->dropStorage();
      }

      void TearDown() override {
        clear();
        disconnect();
      }

      std::shared_ptr<pqxx::lazyconnection> connection;

      std::shared_ptr<StorageImpl> storage;

      std::string pgopt_ =
          "host=localhost port=5432 user=postgres password=mysecretpassword";

      std::string block_store_path =
          (boost::filesystem::temp_directory_path() / "block_store").string();

      // TODO(warchant): IR-1019 hide SQLs under some interface
      const std::string drop_ = R"(
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

      const std::string init_ = R"(
CREATE TABLE IF NOT EXISTS role (
    role_id character varying(32),
    PRIMARY KEY (role_id)
);
CREATE TABLE IF NOT EXISTS domain (
    domain_id character varying(255),
    default_role character varying(32) NOT NULL REFERENCES role(role_id),
    PRIMARY KEY (domain_id)
);
CREATE TABLE IF NOT EXISTS signatory (
    public_key bytea NOT NULL,
    PRIMARY KEY (public_key)
);
CREATE TABLE IF NOT EXISTS account (
    account_id character varying(288),
    domain_id character varying(255) NOT NULL REFERENCES domain,
    quorum int NOT NULL,
    data JSONB,
    PRIMARY KEY (account_id)
);
CREATE TABLE IF NOT EXISTS account_has_signatory (
    account_id character varying(288) NOT NULL REFERENCES account,
    public_key bytea NOT NULL REFERENCES signatory,
    PRIMARY KEY (account_id, public_key)
);
CREATE TABLE IF NOT EXISTS peer (
    public_key bytea NOT NULL,
    address character varying(261) NOT NULL UNIQUE,
    PRIMARY KEY (public_key)
);
CREATE TABLE IF NOT EXISTS asset (
    asset_id character varying(288),
    domain_id character varying(255) NOT NULL REFERENCES domain,
    precision int NOT NULL,
    data json,
    PRIMARY KEY (asset_id)
);
CREATE TABLE IF NOT EXISTS account_has_asset (
    account_id character varying(288) NOT NULL REFERENCES account,
    asset_id character varying(288) NOT NULL REFERENCES asset,
    amount decimal NOT NULL,
    PRIMARY KEY (account_id, asset_id)
);
CREATE TABLE IF NOT EXISTS role_has_permissions (
    role_id character varying(32) NOT NULL REFERENCES role,
    permission_id character varying(45),
    PRIMARY KEY (role_id, permission_id)
);
CREATE TABLE IF NOT EXISTS account_has_roles (
    account_id character varying(288) NOT NULL REFERENCES account,
    role_id character varying(32) NOT NULL REFERENCES role,
    PRIMARY KEY (account_id, role_id)
);
CREATE TABLE IF NOT EXISTS account_has_grantable_permissions (
    permittee_account_id character varying(288) NOT NULL REFERENCES account,
    account_id character varying(288) NOT NULL REFERENCES account,
    permission_id character varying(45),
    PRIMARY KEY (permittee_account_id, account_id, permission_id)
);
CREATE TABLE IF NOT EXISTS height_by_hash (
    hash bytea,
    height text
);
CREATE TABLE IF NOT EXISTS height_by_account_set (
    account_id text,
    height text
);
CREATE TABLE IF NOT EXISTS index_by_creator_height (
    id serial,
    creator_id text,
    height text,
    index text
);
CREATE TABLE IF NOT EXISTS index_by_id_height_asset (
    id text,
    height text,
    asset_id text,
    index text
);
)";
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_FIXTURE_HPP
