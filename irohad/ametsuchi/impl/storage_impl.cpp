/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/storage_impl.hpp"

#include <soci/postgresql/soci-postgresql.h>
#include <boost/format.hpp>

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/impl/mutable_storage_impl.hpp"
#include "ametsuchi/impl/peer_query_wsv.hpp"
#include "ametsuchi/impl/postgres_block_index.hpp"
#include "ametsuchi/impl/postgres_block_query.hpp"
#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_query_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/temporary_wsv_impl.hpp"
#include "backend/protobuf/permissions.hpp"
#include "common/bind.hpp"
#include "common/byteutils.hpp"
#include "converters/protobuf/json_proto_converter.hpp"
#include "postgres_ordering_service_persistent_state.hpp"

namespace {
  void prepareStatements(soci::connection_pool &connections, size_t pool_size) {
    for (size_t i = 0; i != pool_size; i++) {
      soci::session &session = connections.at(i);
      iroha::ametsuchi::PostgresCommandExecutor::prepareStatements(session);
    }
  }

  /**
   * Verify whether postgres supports prepared transactions
   */
  bool preparedTransactionsAvailable(soci::session &sql) {
    int prepared_txs_count = 0;
    sql << "SHOW max_prepared_transactions;", soci::into(prepared_txs_count);
    return prepared_txs_count != 0;
  }

}  // namespace

namespace iroha {
  namespace ametsuchi {
    const char *kCommandExecutorError = "Cannot create CommandExecutorFactory";
    const char *kPsqlBroken = "Connection to PostgreSQL broken: %s";
    const char *kTmpWsv = "TemporaryWsv";

    ConnectionContext::ConnectionContext(
        std::unique_ptr<KeyValueStorage> block_store)
        : block_store(std::move(block_store)) {}

    StorageImpl::StorageImpl(
        std::string block_store_dir,
        PostgresOptions postgres_options,
        std::unique_ptr<KeyValueStorage> block_store,
        std::shared_ptr<soci::connection_pool> connection,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory,
        std::shared_ptr<shared_model::interface::BlockJsonConverter> converter,
        std::shared_ptr<shared_model::interface::PermissionToString>
            perm_converter,
        size_t pool_size,
        bool enable_prepared_blocks)
        : block_store_dir_(std::move(block_store_dir)),
          postgres_options_(std::move(postgres_options)),
          block_store_(std::move(block_store)),
          connection_(std::move(connection)),
          factory_(std::move(factory)),
          converter_(std::move(converter)),
          perm_converter_(std::move(perm_converter)),
          log_(logger::log("StorageImpl")),
          pool_size_(pool_size),
          prepared_blocks_enabled_(enable_prepared_blocks),
          block_is_prepared(false) {
      prepared_block_name_ =
          "prepared_block" + postgres_options_.dbname().value_or("");
      soci::session sql(*connection_);
      // rollback current prepared transaction
      // if there exists any since last session
      if (prepared_blocks_enabled_) {
        rollbackPrepared(sql);
      }
      sql << init_;
      prepareStatements(*connection_, pool_size_);
    }

    expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
    StorageImpl::createTemporaryWsv() {
      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (connection_ == nullptr) {
        return expected::makeError("Connection was closed");
      }
      auto sql = std::make_unique<soci::session>(*connection_);

      return expected::makeValue<std::unique_ptr<TemporaryWsv>>(
          std::make_unique<TemporaryWsvImpl>(
              std::move(sql), factory_, perm_converter_));
    }

    expected::Result<std::unique_ptr<MutableStorage>, std::string>
    StorageImpl::createMutableStorage() {
      boost::optional<shared_model::interface::types::HashType> top_hash;

      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (connection_ == nullptr) {
        return expected::makeError("Connection was closed");
      }

      auto sql = std::make_unique<soci::session>(*connection_);
      // if we create mutable storage, then we intend to mutate wsv
      // this means that any state prepared before that moment is not needed
      // and must be removed to preventy locking
      if (block_is_prepared) {
        rollbackPrepared(*sql);
      }
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
              std::make_shared<PostgresCommandExecutor>(*sql, perm_converter_),
              std::move(sql),
              factory_));
    }

    boost::optional<std::shared_ptr<PeerQuery>> StorageImpl::createPeerQuery()
        const {
      auto wsv = getWsvQuery();
      if (not wsv) {
        return boost::none;
      }
      return boost::make_optional<std::shared_ptr<PeerQuery>>(
          std::make_shared<PeerQueryWsv>(wsv));
    }

    boost::optional<std::shared_ptr<BlockQuery>> StorageImpl::createBlockQuery()
        const {
      auto block_query = getBlockQuery();
      if (not block_query) {
        return boost::none;
      }
      return boost::make_optional(block_query);
    }

    boost::optional<std::shared_ptr<OrderingServicePersistentState>>
    StorageImpl::createOsPersistentState() const {
      log_->info("create ordering service persistent state");
      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (not connection_) {
        log_->info("connection to database is not initialised");
        return boost::none;
      }
      return boost::make_optional<
          std::shared_ptr<OrderingServicePersistentState>>(
          std::make_shared<PostgresOrderingServicePersistentState>(
              std::make_unique<soci::session>(*connection_)));
    }

    boost::optional<std::shared_ptr<QueryExecutor>>
    StorageImpl::createQueryExecutor(
        std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
        std::shared_ptr<shared_model::interface::QueryResponseFactory>
            response_factory) const {
      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (not connection_) {
        log_->info("connection to database is not initialised");
        return boost::none;
      }
      return boost::make_optional<std::shared_ptr<QueryExecutor>>(
          std::make_shared<PostgresQueryExecutor>(
              std::make_unique<soci::session>(*connection_),
              *block_store_,
              std::move(pending_txs_storage),
              converter_,
              std::move(response_factory),
              perm_converter_));
    }

    bool StorageImpl::insertBlock(const shared_model::interface::Block &block) {
      log_->info("create mutable storage");
      auto storageResult = createMutableStorage();
      bool inserted = false;
      storageResult.match(
          [&](expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>
                  &storage) {
            inserted = storage.value->apply(block);
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
              inserted &= mutableStorage.value->apply(*block);
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

    void StorageImpl::reset() {
      log_->info("drop wsv records from db tables");
      soci::session sql(*connection_);
      sql << reset_;

      log_->info("drop blocks from disk");
      block_store_->dropAll();
    }

    void StorageImpl::dropStorage() {
      log_->info("drop storage");
      if (connection_ == nullptr) {
        log_->warn("Tried to drop storage without active connection");
        return;
      }

      if (auto dbname = postgres_options_.dbname()) {
        auto &db = dbname.value();
        std::unique_lock<std::shared_timed_mutex> lock(drop_mutex);
        log_->info("Drop database {}", db);
        freeConnections();
        soci::session sql(soci::postgresql,
                          postgres_options_.optionsStringWithoutDbName());
        // perform dropping
        sql << "DROP DATABASE " + db;
      } else {
        soci::session(*connection_) << drop_;
      }

      // erase blocks
      log_->info("drop block store");
      block_store_->dropAll();
    }

    void StorageImpl::freeConnections() {
      if (connection_ == nullptr) {
        log_->warn("Tried to free connections without active connection");
        return;
      }
      // rollback possible prepared transaction
      if (block_is_prepared) {
        soci::session sql(*connection_);
        rollbackPrepared(sql);
      }
      std::vector<std::shared_ptr<soci::session>> connections;
      for (size_t i = 0; i < pool_size_; i++) {
        connections.push_back(std::make_shared<soci::session>(*connection_));
        connections[i]->close();
        log_->debug("Closed connection {}", i);
      }
      connections.clear();
      connection_.reset();
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
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory,
        std::shared_ptr<shared_model::interface::BlockJsonConverter> converter,
        std::shared_ptr<shared_model::interface::PermissionToString>
            perm_converter,
        size_t pool_size) {
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
      auto db_result = initPostgresConnection(postgres_options, pool_size);
      expected::Result<std::shared_ptr<StorageImpl>, std::string> storage;
      ctx_result.match(
          [&](expected::Value<ConnectionContext> &ctx) {
            db_result.match(
                [&](expected::Value<std::shared_ptr<soci::connection_pool>>
                        &connection) {
                  soci::session sql(*connection.value);
                  bool enable_prepared_transactions =
                      preparedTransactionsAvailable(sql);
                  storage = expected::makeValue(std::shared_ptr<StorageImpl>(
                      new StorageImpl(block_store_dir,
                                      options,
                                      std::move(ctx.value.block_store),
                                      connection.value,
                                      factory,
                                      converter,
                                      perm_converter,
                                      pool_size,
                                      enable_prepared_transactions)));
                },
                [&](expected::Error<std::string> &error) { storage = error; });
          },
          [&](expected::Error<std::string> &error) { storage = error; });
      return storage;
    }

    void StorageImpl::commit(std::unique_ptr<MutableStorage> mutableStorage) {
      auto storage_ptr = std::move(mutableStorage);  // get ownership of storage
      auto storage = static_cast<MutableStorageImpl *>(storage_ptr.get());
      for (const auto &block : storage->block_store_) {
        storeBlock(*block.second);
      }
      *(storage->sql_) << "COMMIT";
      storage->committed = true;
    }

    bool StorageImpl::commitPrepared(
        const shared_model::interface::Block &block) {
      if (not prepared_blocks_enabled_) {
        log_->warn("prepared blocks are not enabled");
        return false;
      }

      if (not block_is_prepared) {
        log_->info("there are no prepared blocks");
        return false;
      }
      log_->info("applying prepared block");

      try {
        std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
        if (not connection_) {
          log_->info("connection to database is not initialised");
          return false;
        }
        soci::session sql(*connection_);
        sql << "COMMIT PREPARED '" + prepared_block_name_ + "';";
        PostgresBlockIndex block_index(sql);
        block_index.index(block);
        block_is_prepared = false;
      } catch (const std::exception &e) {
        log_->warn("failed to apply prepared block {}: {}",
                   block.hash().hex(),
                   e.what());
        return false;
      }

      return storeBlock(block);
    }

    std::shared_ptr<WsvQuery> StorageImpl::getWsvQuery() const {
      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (not connection_) {
        log_->info("connection to database is not initialised");
        return nullptr;
      }
      return std::make_shared<PostgresWsvQuery>(
          std::make_unique<soci::session>(*connection_), factory_);
    }

    std::shared_ptr<BlockQuery> StorageImpl::getBlockQuery() const {
      std::shared_lock<std::shared_timed_mutex> lock(drop_mutex);
      if (not connection_) {
        log_->info("connection to database is not initialised");
        return nullptr;
      }
      return std::make_shared<PostgresBlockQuery>(
          std::make_unique<soci::session>(*connection_),
          *block_store_,
          converter_);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    StorageImpl::on_commit() {
      return notifier_.get_observable();
    }

    void StorageImpl::prepareBlock(std::unique_ptr<TemporaryWsv> wsv) {
      auto &wsv_impl = static_cast<TemporaryWsvImpl &>(*wsv);
      if (not prepared_blocks_enabled_) {
        log_->warn("prepared block are not enabled");
        return;
      }
      if (not block_is_prepared) {
        soci::session &sql = *wsv_impl.sql_;
        try {
          sql << "PREPARE TRANSACTION '" + prepared_block_name_ + "';";
          block_is_prepared = true;
        } catch (const std::exception &e) {
          log_->warn("failed to prepare state: {}", e.what());
        }

        log_->info("state prepared successfully");
      }
    }

    StorageImpl::~StorageImpl() {
      freeConnections();
    }

    void StorageImpl::rollbackPrepared(soci::session &sql) {
      try {
        sql << "ROLLBACK PREPARED '" + prepared_block_name_ + "';";
        block_is_prepared = false;
      } catch (const std::exception &e) {
        log_->info(e.what());
      }
    }

    bool StorageImpl::storeBlock(const shared_model::interface::Block &block) {
      auto json_result = converter_->serialize(block);
      return json_result.match(
          [this, &block](const expected::Value<std::string> &v) {
            block_store_->add(block.height(), stringToBytes(v.value));
            notifier_.get_subscriber().on_next(clone(block));
            return true;
          },
          [this](const expected::Error<std::string> &e) {
            log_->error(e.error);
            return false;
          });
    }

    const std::string &StorageImpl::drop_ = R"(
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
DROP TABLE IF EXISTS position_by_account_asset;
)";

    const std::string &StorageImpl::reset_ = R"(
DELETE FROM account_has_signatory;
DELETE FROM account_has_asset;
DELETE FROM role_has_permissions CASCADE;
DELETE FROM account_has_roles;
DELETE FROM account_has_grantable_permissions CASCADE;
DELETE FROM account;
DELETE FROM asset;
DELETE FROM domain;
DELETE FROM signatory;
DELETE FROM peer;
DELETE FROM role;
DELETE FROM position_by_hash;
DELETE FROM height_by_account_set;
DELETE FROM index_by_creator_height;
DELETE FROM position_by_account_asset;
)";

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
CREATE TABLE IF NOT EXISTS position_by_hash (
    hash varchar,
    height text,
    index text
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
CREATE TABLE IF NOT EXISTS position_by_account_asset (
    account_id text,
    asset_id text,
    height text,
    index text
);
)";
  }  // namespace ametsuchi
}  // namespace iroha
