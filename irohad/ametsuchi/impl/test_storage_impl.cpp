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
                              std::move(ctx->pg_nontx),
                              std::move(ctx->wsv)));
    }

    TestStorageImpl::TestStorageImpl(std::string block_store_dir,
                                     std::string redis_host,
                                     size_t redis_port,
                                     std::string postgres_options,
                                     std::unique_ptr<FlatFile> block_store,
                                     std::unique_ptr<cpp_redis::redis_client> index,
                                     std::unique_ptr<pqxx::lazyconnection> wsv_connection,
                                     std::unique_ptr<pqxx::nontransaction> wsv_transaction,
                                     std::unique_ptr<WsvQuery> wsv)
        : StorageImpl(block_store_dir,
                      redis_host,
                      redis_port,
                      postgres_options,
                      std::move(block_store),
                      std::move(index),
                      std::move(wsv_connection),
                      std::move(wsv_transaction),
                      std::move(wsv)) {
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

    rxcpp::observable<model::Transaction> TestStorageImpl::getAccountTransactions(
        std::string account_id) {
      return StorageImpl::getAccountTransactions(account_id);
    }

    rxcpp::observable<model::Transaction> TestStorageImpl::getAccountAssetTransactions(
        std::string account_id, std::string asset_id) {
      return StorageImpl::getAccountAssetTransactions(account_id, asset_id);
    }

    rxcpp::observable<model::Block> TestStorageImpl::getBlocks(uint32_t height,
                                                               uint32_t count) {
      return StorageImpl::getBlocks(height, count);
    };

    rxcpp::observable<model::Block> TestStorageImpl::getBlocksFrom(uint32_t height) {
      return StorageImpl::getBlocksFrom(height);
    };

    rxcpp::observable<model::Block> TestStorageImpl::getTopBlocks(uint32_t count) {
      return StorageImpl::getTopBlocks(count);
    };

    nonstd::optional<model::Account> TestStorageImpl::getAccount(
        const std::string &account_id) {
      return StorageImpl::getAccount(account_id);
    }

    nonstd::optional<std::vector<ed25519::pubkey_t>> TestStorageImpl::getSignatories(
        const std::string &account_id) {
      return StorageImpl::getSignatories(account_id);
    }

    nonstd::optional<model::Asset> TestStorageImpl::getAsset(
        const std::string &asset_id) {
      return StorageImpl::getAsset(asset_id);
    }

    nonstd::optional<model::AccountAsset> TestStorageImpl::getAccountAsset(
        const std::string &account_id, const std::string &asset_id) {
      return StorageImpl::getAccountAsset(account_id, asset_id);
    }

    nonstd::optional<std::vector<model::Peer>> TestStorageImpl::getPeers() {
      return StorageImpl::getPeers();
    }

  }  // namespace ametsuchi
}  // namespace iroha
