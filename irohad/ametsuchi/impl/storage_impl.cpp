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

#include "ametsuchi/impl/storage_impl.hpp"

#include <boost/format.hpp>
#include <soci/postgresql/soci-postgresql.h>

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/impl/mutable_storage_impl.hpp"
#include "ametsuchi/impl/postgres_block_query.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/temporary_wsv_impl.hpp"
#include "backend/protobuf/permissions.hpp"
#include "converters/protobuf/json_proto_converter.hpp"
#include "postgres_ordering_service_persistent_state.hpp"

namespace iroha {
  namespace ametsuchi {

    const char *kCommandExecutorError = "Cannot create CommandExecutorFactory";
    const char *kPsqlBroken = "Connection to PostgreSQL broken: %s";
    const char *kTmpWsv = "TemporaryWsv";

    ConnectionContext::ConnectionContext(
        std::unique_ptr<KeyValueStorage> block_store)
        : block_store(std::move(block_store)) {}

    StorageImpl::StorageImpl(std::string block_store_dir,
                             PostgresOptions postgres_options,
                             std::unique_ptr<KeyValueStorage> block_store,
                             std::shared_ptr<soci::connection_pool> connection,
                             std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : block_store_dir_(std::move(block_store_dir)),
          postgres_options_(std::move(postgres_options)),
          block_store_(std::move(block_store)),
          connection_(connection),
          factory_(factory),
          log_(logger::log("StorageImpl")) {}

    expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
    StorageImpl::createTemporaryWsv() {
      auto sql = std::make_unique<soci::session>(*connection_);

      return expected::makeValue<std::unique_ptr<TemporaryWsv>>(
          std::make_unique<TemporaryWsvImpl>(std::move(sql), factory_));
    }

    expected::Result<std::unique_ptr<MutableStorage>, std::string>
    StorageImpl::createMutableStorage() {
      boost::optional<shared_model::interface::types::HashType> top_hash;

      auto sql = std::make_unique<soci::session>(*connection_);
      auto block_result = getBlockQuery()->getTopBlock();
      return expected::makeValue<std::unique_ptr<MutableStorage>>(
          std::make_unique<MutableStorageImpl>(
              block_result.match(
                  [](expected::Value<
                      std::shared_ptr<shared_model::interface::Block>> &block) {
                    return block.value->hash();
                  },
                  [](expected::Error<std::string> &) {
                    return shared_model::interface::types::HashType("");
                  }),
              std::move(sql),
              factory_));
    }

    bool StorageImpl::insertBlock(const shared_model::interface::Block &block) {
      log_->info("create mutable storage");
      auto storageResult = createMutableStorage();
      bool inserted = false;
      storageResult.match(
          [&](expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>
                  &storage) {
            inserted =
                storage.value->apply(block,
                                     [](const auto &current_block,
                                        auto &query,
                                        const auto &top_hash) { return true; });
            log_->info("block inserted: {}", inserted);
            commit(std::move(storage.value));
          },
          [&](expected::Error<std::string> &error) {
            log_->error(error.error);
          });

      return inserted;
    }

    bool StorageImpl::insertBlocks(
        const std::vector<std::shared_ptr<shared_model::interface::Block>>
            &blocks) {
      log_->info("create mutable storage");
      bool inserted = true;
      auto storageResult = createMutableStorage();
      storageResult.match(
          [&](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                  &mutableStorage) {
            std::for_each(blocks.begin(), blocks.end(), [&](auto block) {
              inserted &= mutableStorage.value->apply(
                  *block, [](const auto &block, auto &query, const auto &hash) {
                    return true;
                  });
            });
            commit(std::move(mutableStorage.value));
          },
          [&](iroha::expected::Error<std::string> &error) {
            log_->error(error.error);
            inserted = false;
          });

      log_->info("insert blocks finished");
      return inserted;
    }

    void StorageImpl::dropStorage() {
      log_->info("Drop ledger");
      auto drop = R"(
DROP TABLE IF EXISTS account_has_signatory;
DROP TABLE IF EXISTS account_has_asset;
DROP TABLE IF EXISTS role_has_permissions CASCADE;
DROP TABLE IF EXISTS account_has_roles;
DROP TABLE IF EXISTS account_has_grantable_permissions CASCADE;
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

      // erase db
      log_->info("drop db");

      soci::session sql(*connection_);
      sql << drop;
      sql << init_;

      // erase blocks
      log_->info("drop block store");
      block_store_->dropAll();
    }

    expected::Result<bool, std::string> StorageImpl::createDatabaseIfNotExist(
        const std::string &dbname,
        const std::string &options_str_without_dbname) {
      try {
        soci::session sql(soci::postgresql, options_str_without_dbname);

        int size;
        std::string name = dbname;

        sql << "SELECT count(datname) FROM pg_catalog.pg_database WHERE "
               "datname = :dbname",
            soci::into(size), soci::use(name);

        if (size == 0) {
          std::string query = "CREATE DATABASE ";
          query += dbname;
          sql << query;
          return expected::makeValue(true);
        }
        return expected::makeValue(false);
      } catch (std::exception &e) {
        return expected::makeError<std::string>(
            std::string("Connection to PostgreSQL broken: ") + e.what());
      }
    }

    expected::Result<ConnectionContext, std::string>
    StorageImpl::initConnections(std::string block_store_dir) {
      auto log_ = logger::log("StorageImpl:initConnection");
      log_->info("Start storage creation");

      auto block_store = FlatFile::create(block_store_dir);
      if (not block_store) {
        return expected::makeError(
            (boost::format("Cannot create block store in %s") % block_store_dir)
                .str());
      }
      log_->info("block store created");

      return expected::makeValue(ConnectionContext(std::move(*block_store)));
    }

    expected::Result<std::shared_ptr<soci::connection_pool>, std::string>
    StorageImpl::initPostgresConnection(std::string &options_str,
                                        size_t pool_size) {
      auto pool = std::make_shared<soci::connection_pool>(pool_size);

      for (size_t i = 0; i != pool_size; i++) {
        soci::session &session = pool->at(i);
        session.open(soci::postgresql, options_str);
      }
      return expected::makeValue(pool);
    };

    expected::Result<std::shared_ptr<StorageImpl>, std::string>
    StorageImpl::create(
        std::string block_store_dir,
        std::string postgres_options,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory>
            factory) {
      boost::optional<std::string> string_res = boost::none;

      PostgresOptions options(postgres_options);

      // create database if
      options.dbname() | [&options, &string_res](const std::string &dbname) {
        createDatabaseIfNotExist(dbname, options.optionsStringWithoutDbName())
            .match([](expected::Value<bool> &val) {},
                   [&string_res](expected::Error<std::string> &error) {
                     string_res = error.error;
                   });
      };

      if (string_res) {
        return expected::makeError(string_res.value());
      }

      auto ctx_result = initConnections(block_store_dir);
      auto db_result = initPostgresConnection(postgres_options);
      expected::Result<std::shared_ptr<StorageImpl>, std::string> storage;
      ctx_result.match(
          [&](expected::Value<ConnectionContext> &ctx) {
            db_result.match(
                [&](expected::Value<std::shared_ptr<soci::connection_pool>>
                        &connection) {
                  storage = expected::makeValue(std::shared_ptr<StorageImpl>(
                      new StorageImpl(block_store_dir,
                                      options,
                                      std::move(ctx.value.block_store),
                                      connection.value,
                                      factory)));
                },
                [&](expected::Error<std::string> &error) { storage = error; });
          },
          [&](expected::Error<std::string> &error) { storage = error; });
      return storage;
    }

    void StorageImpl::commit(std::unique_ptr<MutableStorage> mutableStorage) {
      std::unique_lock<std::shared_timed_mutex> write(rw_lock_);
      auto storage_ptr = std::move(mutableStorage);  // get ownership of storage
      auto storage = static_cast<MutableStorageImpl *>(storage_ptr.get());
      for (const auto &block : storage->block_store_) {
        block_store_->add(
            block.first,
            stringToBytes(shared_model::converters::protobuf::modelToJson(
                *std::static_pointer_cast<shared_model::proto::Block>(
                    block.second))));
        notifier_.get_subscriber().on_next(block.second);
      }

      *(storage->sql_) << "COMMIT";
      storage->committed = true;
    }

    std::shared_ptr<WsvQuery> StorageImpl::getWsvQuery() const {
      auto sql = std::make_unique<soci::session>(*connection_);
      return std::make_shared<PostgresWsvQuery>(std::move(sql), factory_);
    }

    std::shared_ptr<BlockQuery> StorageImpl::getBlockQuery() const {
      auto sql = std::make_unique<soci::session>(
          soci::postgresql, postgres_options_.optionsString());

      return std::make_shared<PostgresBlockQuery>(std::move(sql),
                                                  *block_store_);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    StorageImpl::on_commit() {
      return notifier_.get_observable();
    }

    const std::string &StorageImpl::init_ =
        R"(
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
    public_key varchar NOT NULL,
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
    public_key varchar NOT NULL REFERENCES signatory,
    PRIMARY KEY (account_id, public_key)
);
CREATE TABLE IF NOT EXISTS peer (
    public_key varchar NOT NULL,
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
    permission bit()"
        + std::to_string(shared_model::interface::RolePermissionSet::size())
        + R"() NOT NULL,
    PRIMARY KEY (role_id)
);
CREATE TABLE IF NOT EXISTS account_has_roles (
    account_id character varying(288) NOT NULL REFERENCES account,
    role_id character varying(32) NOT NULL REFERENCES role,
    PRIMARY KEY (account_id, role_id)
);
CREATE TABLE IF NOT EXISTS account_has_grantable_permissions (
    permittee_account_id character varying(288) NOT NULL REFERENCES account,
    account_id character varying(288) NOT NULL REFERENCES account,
    permission bit()"
        + std::to_string(
              shared_model::interface::GrantablePermissionSet::size())
        + R"() NOT NULL,
    PRIMARY KEY (permittee_account_id, account_id)
);
CREATE TABLE IF NOT EXISTS height_by_hash (
    hash varchar,
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
  }  // namespace ametsuchi
}  // namespace iroha
