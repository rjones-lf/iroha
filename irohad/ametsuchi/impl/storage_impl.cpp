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
#include "ametsuchi/impl/flat_file/flat_file.hpp"  // for FlatFile
#include "ametsuchi/impl/mutable_storage_impl.hpp"
#include "ametsuchi/impl/postgres_block_query.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/temporary_wsv_impl.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "converters/protobuf/json_proto_converter.hpp"
#include "interfaces/common_objects/types.hpp"
#include "model/converters/json_common.hpp"
#include "model/execution/command_executor_factory.hpp"  // for CommandExecutorFactory
#include "postgres_ordering_service_persistent_state.hpp"

namespace iroha {
  namespace ametsuchi {

    const char *kCommandExecutorError = "Cannot create CommandExecutorFactory";
    const char *kPsqlBroken = "Connection to PostgreSQL broken: %s";
    const char *kTmpWsv = "TemporaryWsv";

    ConnectionContext::ConnectionContext(
        std::unique_ptr<FlatFile> block_store,
        std::unique_ptr<pqxx::lazyconnection> pg_lazy,
        std::unique_ptr<pqxx::nontransaction> pg_nontx)
        : block_store(std::move(block_store)),
          pg_lazy(std::move(pg_lazy)),
          pg_nontx(std::move(pg_nontx)) {}

    StorageImpl::~StorageImpl() {
      wsv_transaction_->commit();
      wsv_connection_->disconnect();
      log_->info("PostgresQL connection closed");
    }

    StorageImpl::StorageImpl(
        std::string block_store_dir,
        std::string postgres_options,
        std::unique_ptr<FlatFile> block_store,
        std::unique_ptr<pqxx::lazyconnection> wsv_connection,
        std::unique_ptr<pqxx::nontransaction> wsv_transaction)
        : block_store_dir_(std::move(block_store_dir)),
          postgres_options_(std::move(postgres_options)),
          block_store_(std::move(block_store)),
          wsv_connection_(std::move(wsv_connection)),
          wsv_transaction_(std::move(wsv_transaction)),
          wsv_(std::make_shared<PostgresWsvQuery>(*wsv_transaction_)),
          blocks_(std::make_shared<PostgresBlockQuery>(*wsv_transaction_,
                                                       *block_store_)) {
      log_ = logger::log("StorageImpl");

      wsv_transaction_->exec(init_);
      wsv_transaction_->exec(
          "SET SESSION CHARACTERISTICS AS TRANSACTION READ ONLY;");
    }

    expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
    StorageImpl::createTemporaryWsv() {
      auto command_executors = model::CommandExecutorFactory::create();
      if (not command_executors.has_value()) {
        return expected::makeError(kCommandExecutorError);
      }

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options_);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        return expected::makeError(
            (boost::format(kPsqlBroken) % e.what()).str());
      }
      auto wsv_transaction =
          std::make_unique<pqxx::nontransaction>(*postgres_connection, kTmpWsv);

      return expected::makeValue<std::unique_ptr<TemporaryWsv>>(
          std::make_unique<TemporaryWsvImpl>(
              std::move(postgres_connection),
              std::move(wsv_transaction),
              std::move(command_executors.value())));
    }

    expected::Result<std::unique_ptr<MutableStorage>, std::string>
    StorageImpl::createMutableStorage() {
      auto command_executors = model::CommandExecutorFactory::create();
      if (not command_executors.has_value()) {
        return expected::makeError(kCommandExecutorError);
      }

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options_);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        return expected::makeError(
            (boost::format(kPsqlBroken) % e.what()).str());
      }
      auto wsv_transaction =
          std::make_unique<pqxx::nontransaction>(*postgres_connection, kTmpWsv);

      nonstd::optional<shared_model::interface::types::HashType> top_hash;

      blocks_->getTopBlocks(1)
          .subscribe_on(rxcpp::observe_on_new_thread())
          .as_blocking()
          .subscribe([&top_hash](auto block) { top_hash = block->hash(); });

      return expected::makeValue<std::unique_ptr<MutableStorage>>(
          std::make_unique<MutableStorageImpl>(
              top_hash.value_or(shared_model::interface::types::HashType("")),
              std::move(postgres_connection),
              std::move(wsv_transaction),
              std::move(command_executors.value())));
    }

    bool StorageImpl::insertBlock(model::Block block) {
      log_->info("create mutable storage");
      auto storageResult = createMutableStorage();
      bool inserted = false;
      auto old_block = shared_model::proto::from_old(block);
      storageResult.match(
          [&](expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>
                  &storage) {
            inserted =
                storage.value->apply(old_block,
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

    bool StorageImpl::insertBlocks(const std::vector<model::Block> &blocks) {
      log_->info("create mutable storage");
      bool inserted = true;
      auto storageResult = createMutableStorage();
      storageResult.match(
          [&](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                  &mutableStorage) {
            std::for_each(blocks.begin(), blocks.end(), [&](auto block) {
              auto old_block = shared_model::proto::from_old(block);
              inserted &= mutableStorage.value->apply(
                  old_block,
                  [](const auto &block, auto &query, const auto &hash) {
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

      // erase db
      log_->info("drop dp");
      pqxx::connection connection(postgres_options_);
      pqxx::work txn(connection);
      txn.exec(drop);
      txn.commit();

      pqxx::work init_txn(connection);
      init_txn.exec(init_);
      init_txn.commit();

      // erase blocks
      log_->info("drop block store");
      block_store_->dropAll();
    }

    expected::Result<ConnectionContext, std::string>
    StorageImpl::initConnections(std::string block_store_dir,
                                 std::string postgres_options) {
      auto log_ = logger::log("StorageImpl:initConnection");
      log_->info("Start storage creation");

      auto block_store = FlatFile::create(block_store_dir);
      if (not block_store) {
        return expected::makeError(
            (boost::format("Cannot create block store in %s") % block_store_dir)
                .str());
      }
      log_->info("block store created");

      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        return expected::makeError(
            (boost::format(kPsqlBroken) % e.what()).str());
      }
      log_->info("connection to PostgreSQL completed");

      auto wsv_transaction = std::make_unique<pqxx::nontransaction>(
          *postgres_connection, "Storage");
      log_->info("transaction to PostgreSQL initialized");

      return expected::makeValue(
          ConnectionContext(std::move(*block_store),
                            std::move(postgres_connection),
                            std::move(wsv_transaction)));
    }

    expected::Result<std::shared_ptr<StorageImpl>, std::string>
    StorageImpl::create(std::string block_store_dir,
                        std::string postgres_options) {
      auto ctx_result = initConnections(block_store_dir, postgres_options);
      expected::Result<std::shared_ptr<StorageImpl>, std::string> storage;
      ctx_result.match(
          [&](expected::Value<ConnectionContext> &ctx) {
            storage = expected::makeValue(std::shared_ptr<StorageImpl>(
                new StorageImpl(block_store_dir,
                                postgres_options,
                                std::move(ctx.value.block_store),
                                std::move(ctx.value.pg_lazy),
                                std::move(ctx.value.pg_nontx))));
          },
          [&](expected::Error<std::string> &error) { storage = error; });
      return storage;
    }

    void StorageImpl::commit(std::unique_ptr<MutableStorage> mutableStorage) {
      std::unique_lock<std::shared_timed_mutex> write(rw_lock_);
      auto storage_ptr = std::move(mutableStorage);  // get ownership of storage
      auto storage = static_cast<MutableStorageImpl *>(storage_ptr.get());
      for (const auto &block : storage->block_store_) {
        auto old_block = *std::unique_ptr<model::Block>(block.second->makeOldModel());
        block_store_->add(block.first,
                          stringToBytes(model::converters::jsonToString(
                              serializer_.serialize(old_block))));
      }

      storage->transaction_->exec("COMMIT;");
      storage->committed = true;
    }

    std::shared_ptr<WsvQuery> StorageImpl::getWsvQuery() const {
      return wsv_;
    }

    std::shared_ptr<BlockQuery> StorageImpl::getBlockQuery() const {
      return blocks_;
    }
  }  // namespace ametsuchi
}  // namespace iroha
