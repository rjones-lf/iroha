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

#include <utility>
#include "ametsuchi/impl/test_storage_impl.hpp"
#include "common/files.hpp"

namespace iroha {
  namespace ametsuchi {

    std::shared_ptr<TestStorageImpl> TestStorageImpl::create(
        std::string block_store_dir,
        std::string redis_host,
        std::size_t redis_port,
        std::string postgres_options) {

      auto ctx = initConnections(block_store_dir,
                                 redis_host,
                                 redis_port,
                                 postgres_options);

      return std::shared_ptr<TestStorageImpl>(
          new TestStorageImpl(block_store_dir,
                              redis_host, redis_port,
                              postgres_options,
                              std::move(ctx->block_store),
                              std::move(ctx->index),
                              std::move(ctx->pg_lazy),
                              std::move(ctx->pg_nontx)));
    }

    TestStorageImpl::TestStorageImpl(
        std::string block_store_dir, std::string redis_host, size_t redis_port,
        std::string postgres_options, std::unique_ptr<FlatFile> block_store,
        std::unique_ptr<cpp_redis::redis_client> index,
        std::unique_ptr<pqxx::lazyconnection> wsv_connection,
        std::unique_ptr<pqxx::nontransaction> wsv_transaction)
        : StorageImpl(block_store_dir, redis_host, redis_port, postgres_options,
                      std::move(block_store), std::move(index),
                      std::move(wsv_connection), std::move(wsv_transaction)) {
      log_ = logger::log("TestStorage");
    }

    bool TestStorageImpl::insertBlock(model::Block block) {
      log_->info("create mutable storage");
      auto storage = createMutableStorage();
      auto inserted =
          storage->apply(block, [](const auto &current_block, auto &query,
                                   const auto &top_hash) { return true; });
      log_->info("block inserted: {}", inserted);
      commit(std::move(storage));
      return inserted;
    }

    void TestStorageImpl::dropStorage() {
      log_->info("Drop ledger");
      auto drop =
          "DROP TABLE IF EXISTS account_has_asset;\n"
              "DROP TABLE IF EXISTS account_has_signatory;\n"
              "DROP TABLE IF EXISTS peer;\n"
              "DROP TABLE IF EXISTS account;\n"
              "DROP TABLE IF EXISTS exchange;\n"
              "DROP TABLE IF EXISTS asset;\n"
              "DROP TABLE IF EXISTS domain;\n"
              "DROP TABLE IF EXISTS signatory;";

      // erase db
      log_->info("drop dp");
      pqxx::connection connection(postgres_options_);
      pqxx::work txn(connection);
      txn.exec(drop);
      txn.commit();

      pqxx::connection init_connection(postgres_options_);
      pqxx::work init_txn(connection);
      init_txn.exec(init_);
      init_txn.commit();

      // erase tx index
      log_->info("drop redis");
      cpp_redis::redis_client client;
      client.connect(redis_host_, redis_port_);
      client.flushall();
      client.sync_commit();

      // erase blocks
      log_->info("drop block store");
      remove_all(block_store_dir_);
    }

    // ----------| StorageImpl |----------

    std::unique_ptr<TemporaryWsv> TestStorageImpl::createTemporaryWsv() {
      return StorageImpl::createTemporaryWsv();
    }

    std::unique_ptr<MutableStorage> TestStorageImpl::createMutableStorage() {
      return StorageImpl::createMutableStorage();
    }

    void TestStorageImpl::commit(std::unique_ptr<MutableStorage> mutableStorage) {
      return StorageImpl::commit(std::move(mutableStorage));
    }

    std::shared_ptr<WsvQuery> TestStorageImpl::getWsvQuery() const {
      return StorageImpl::getWsvQuery();
    }

    std::shared_ptr<BlockQuery> TestStorageImpl::getBlockQuery() const {
      return StorageImpl::getBlockQuery();
    }
  }  // namespace ametsuchi
}  // namespace iroha
