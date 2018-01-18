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

#include "ametsuchi/impl/storage_impl.hpp"

#include "ametsuchi/impl/mutable_storage_impl.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/redis_block_query.hpp"
#include "ametsuchi/impl/temporary_wsv_impl.hpp"
#include "model/converters/json_common.hpp"

namespace iroha {
  namespace ametsuchi {

    StorageImpl::StorageImpl(
        std::string block_store_dir,
        std::string redis_host,
        std::size_t redis_port,
        std::string postgres_options,
        std::unique_ptr<FlatFile> block_store,
        std::unique_ptr<cpp_redis::client> index,
        std::unique_ptr<pqxx::lazyconnection> wsv_connection,
        std::unique_ptr<pqxx::nontransaction> wsv_transaction)
        : block_store_dir_(std::move(block_store_dir)),
          redis_host_(std::move(redis_host)),
          redis_port_(redis_port),
          postgres_options_(std::move(postgres_options)),
          block_store_(std::move(block_store)),
          index_(std::move(index)),
          wsv_connection_(std::move(wsv_connection)),
          wsv_transaction_(std::move(wsv_transaction)),
          wsv_(std::make_shared<PostgresWsvQuery>(*wsv_transaction_)),
          blocks_(std::make_shared<RedisBlockQuery>(*index_, *block_store_)) {
      log_ = logger::log("StorageImpl");

      wsv_transaction_->exec(init_);
      wsv_transaction_->exec(
          "SET SESSION CHARACTERISTICS AS TRANSACTION READ ONLY;");
    }

    std::unique_ptr<TemporaryWsv> StorageImpl::createTemporaryWsv() {
      auto command_executors = model::CommandExecutorFactory::create();
      if (not command_executors.has_value()) {
        log_->error("Cannot create CommandExecutorFactory");
        return nullptr;
      }

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options_);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        log_->error("Connection to PostgreSQL broken: {}", e.what());
        return nullptr;
      }
      auto wsv_transaction = std::make_unique<pqxx::nontransaction>(
          *postgres_connection, "TemporaryWsv");

      return std::make_unique<TemporaryWsvImpl>(
          std::move(postgres_connection),
          std::move(wsv_transaction),
          std::move(command_executors.value()));
    }

    std::unique_ptr<MutableStorage> StorageImpl::createMutableStorage() {
      auto command_executors = model::CommandExecutorFactory::create();
      if (not command_executors.has_value()) {
        log_->error("Cannot create CommandExecutorFactory");
        return nullptr;
      }

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options_);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        log_->error("Connection to PostgreSQL broken: {}", e.what());
        return nullptr;
      }
      auto wsv_transaction = std::make_unique<pqxx::nontransaction>(
          *postgres_connection, "TemporaryWsv");

      auto index = std::make_unique<cpp_redis::client>();
      try {
        index->connect(redis_host_, redis_port_);
      } catch (const cpp_redis::redis_error &e) {
        log_->error("Connection to Redis broken: {}", e.what());
        return nullptr;
      }

      nonstd::optional<hash256_t> top_hash;

      blocks_->getTopBlocks(1)
          .subscribe_on(rxcpp::observe_on_new_thread())
          .as_blocking()
          .subscribe([&top_hash](auto block) { top_hash = block.hash; });

      return std::make_unique<MutableStorageImpl>(
          top_hash.value_or(hash256_t{}),
          std::move(index),
          std::move(postgres_connection),
          std::move(wsv_transaction),
          std::move(command_executors.value()));
    }

    bool StorageImpl::insertBlock(model::Block block) {
      log_->info("create mutable storage");
      auto storage = createMutableStorage();
      auto inserted = storage->apply(
          block,
          [](const auto &current_block, auto &query, const auto &top_hash) {
            return true;
          });
      log_->info("block inserted: {}", inserted);
      commit(std::move(storage));
      return inserted;
    }

    void StorageImpl::dropStorage() {
      log_->info("Drop ledger");
      auto drop = R"(
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

      // erase db
      log_->info("drop dp");
      pqxx::connection connection(postgres_options_);
      pqxx::work txn(connection);
      txn.exec(drop);
      txn.commit();

      pqxx::work init_txn(connection);
      init_txn.exec(init_);
      init_txn.commit();

      // erase tx index
      log_->info("drop redis");
      cpp_redis::client client;
      client.connect(redis_host_, redis_port_);
      client.flushall();
      client.sync_commit();

      // erase blocks
      log_->info("drop block store");
      block_store_->dropAll();
    }

    nonstd::optional<ConnectionContext> StorageImpl::initConnections(
        std::string block_store_dir,
        std::string redis_host,
        std::size_t redis_port,
        std::string postgres_options) {
      auto log_ = logger::log("StorageImpl:initConnection");
      log_->info("Start storage creation");

      auto block_store = FlatFile::create(block_store_dir);
      if (not block_store) {
        log_->error("Cannot create block store in {}", block_store_dir);
        return nonstd::nullopt;
      }
      log_->info("block store created");

      auto index = std::make_unique<cpp_redis::client>();
      try {
        index->connect(redis_host, redis_port);
      } catch (const cpp_redis::redis_error &e) {
        log_->error(
            "Connection {}:{} with Redis broken", redis_host, redis_port);
        return nonstd::nullopt;
      }
      log_->info("connection to Redis completed");

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        log_->error("Cannot with PostgreSQL broken: {}", e.what());
        return nonstd::nullopt;
      }
      log_->info("connection to PostgreSQL completed");

      auto wsv_transaction = std::make_unique<pqxx::nontransaction>(
          *postgres_connection, "Storage");
      log_->info("transaction to PostgreSQL initialized");

      return nonstd::make_optional<ConnectionContext>(
          std::move(*block_store),
          std::move(index),
          std::move(postgres_connection),
          std::move(wsv_transaction));
    }

    std::shared_ptr<StorageImpl> StorageImpl::create(
        std::string block_store_dir,
        std::string redis_host,
        std::size_t redis_port,
        std::string postgres_options) {
      auto ctx = initConnections(
          block_store_dir, redis_host, redis_port, postgres_options);
      if (not ctx.has_value()) {
        return nullptr;
      }

      return std::shared_ptr<StorageImpl>(
          new StorageImpl(block_store_dir,
                          redis_host,
                          redis_port,
                          postgres_options,
                          std::move(ctx->block_store),
                          std::move(ctx->index),
                          std::move(ctx->pg_lazy),
                          std::move(ctx->pg_nontx)));
    }

    void StorageImpl::commit(std::unique_ptr<MutableStorage> mutableStorage) {
      std::unique_lock<std::shared_timed_mutex> write(rw_lock_);
      auto storage_ptr = std::move(mutableStorage);  // get ownership of storage
      auto storage = static_cast<MutableStorageImpl *>(storage_ptr.get());
      for (const auto &block : storage->block_store_) {
        block_store_->add(block.first,
                          stringToBytes(model::converters::jsonToString(
                              serializer_.serialize(block.second))));
      }
      storage->index_->exec();
      storage->index_->sync_commit();

      storage->transaction_->exec("COMMIT;");
      storage->committed = true;
    }

    std::shared_ptr<WsvQuery> StorageImpl::getWsvQuery() const { return wsv_; }

    std::shared_ptr<BlockQuery> StorageImpl::getBlockQuery() const {
      return blocks_;
    }
  }  // namespace ametsuchi
}  // namespace iroha
