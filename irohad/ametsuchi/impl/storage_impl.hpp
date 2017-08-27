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

#ifndef IROHA_STORAGE_IMPL_HPP
#define IROHA_STORAGE_IMPL_HPP

#include <cpp_redis/cpp_redis>
#include <nonstd/optional.hpp>
#include <pqxx/pqxx>
#include <shared_mutex>
#include <cmath>
#include "model/converters/json_block_factory.hpp"
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/storage.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    struct ConnectionContext {
      ConnectionContext(std::unique_ptr<FlatFile> block_store,
                        std::unique_ptr<cpp_redis::redis_client> index,
                        std::unique_ptr<pqxx::lazyconnection> pg_lazy,
                        std::unique_ptr<pqxx::nontransaction> pg_nontx,
                        std::unique_ptr<WsvQuery> wsv)
          : block_store(std::move(block_store)),
            index(std::move(index)),
            pg_lazy(std::move(pg_lazy)),
            pg_nontx(std::move(pg_nontx)),
            wsv(std::move(wsv)) {
      }

      std::unique_ptr<FlatFile> block_store;
      std::unique_ptr<cpp_redis::redis_client> index;
      std::unique_ptr<pqxx::lazyconnection> pg_lazy;
      std::unique_ptr<pqxx::nontransaction> pg_nontx;
      std::unique_ptr<WsvQuery> wsv;
    };

    class StorageImpl : public Storage {
     protected:
      static nonstd::optional<ConnectionContext>
      initConnections(std::string block_store_dir,
                      std::string redis_host,
                      std::size_t redis_port,
                      std::string postgres_options);

     public:
      static std::shared_ptr<StorageImpl> create(
          std::string block_store_dir, std::string redis_host,
          std::size_t redis_port, std::string postgres_connection);

      std::unique_ptr<TemporaryWsv> createTemporaryWsv() override;
      std::unique_ptr<MutableStorage> createMutableStorage() override;
      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      rxcpp::observable<model::Transaction> getAccountTransactions(
          std::string account_id) override;
      rxcpp::observable<model::Transaction> getAccountAssetTransactions(
          std::string account_id, std::string asset_id) override;
      rxcpp::observable<model::Block> getBlocks(uint32_t height,
                                                uint32_t count) override;
      rxcpp::observable<model::Block> getBlocksFrom(uint32_t height) override;
      rxcpp::observable<model::Block> getTopBlocks(uint32_t count) override;

      nonstd::optional<model::Account> getAccount(
          const std::string &account_id) override;
      nonstd::optional<std::vector<ed25519::pubkey_t>> getSignatories(
          const std::string &account_id) override;
      nonstd::optional<model::Asset> getAsset(
          const std::string &asset_id) override;
      nonstd::optional<model::AccountAsset> getAccountAsset(
          const std::string &account_id, const std::string &asset_id) override;
      nonstd::optional<std::vector<model::Peer>> getPeers() override;

     protected:

      StorageImpl(std::string block_store_dir,
                  std::string redis_host,
                  std::size_t redis_port,
                  std::string postgres_options,
                  std::unique_ptr<FlatFile> block_store,
                  std::unique_ptr<cpp_redis::redis_client> index,
                  std::unique_ptr<pqxx::lazyconnection> wsv_connection,
                  std::unique_ptr<pqxx::nontransaction> wsv_transaction,
                  std::unique_ptr<WsvQuery> wsv);

      /**
       * Folder with raw blocks
       */
      const std::string block_store_dir_;

      // db info
      const std::string redis_host_;
      const std::size_t redis_port_;
      const std::string postgres_options_;

     private:
      std::unique_ptr<FlatFile> block_store_;

      /**
       * Redis connection
       */
      std::unique_ptr<cpp_redis::redis_client> index_;

      /**
       * Pg connection with direct transaction management
       */
      std::unique_ptr<pqxx::lazyconnection> wsv_connection_;

      std::unique_ptr<pqxx::nontransaction> wsv_transaction_;

      std::unique_ptr<WsvQuery> wsv_;

      model::converters::JsonBlockFactory serializer_;

      // Allows multiple readers and a single writer
      std::shared_timed_mutex rw_lock_;

      logger::Logger log_;

     protected:
      const std::string init_ =
          "CREATE TABLE IF NOT EXISTS domain (\n"
          "    domain_id character varying(164),\n"
          "    open bool NOT NULL DEFAULT TRUE,\n"
          "    PRIMARY KEY (domain_id)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS signatory (\n"
          "    public_key bytea NOT NULL,\n"
          "    PRIMARY KEY (public_key)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS account (\n"
          "    account_id character varying(197),    \n"
          "    domain_id character varying(164) NOT NULL REFERENCES domain,\n"
          "    master_key bytea NOT NULL REFERENCES signatory(public_key),\n"
          "    quorum int NOT NULL,\n"
          "    status int NOT NULL DEFAULT 0,    \n"
          "    transaction_count int NOT NULL DEFAULT 0, \n"
          "    permissions bit varying NOT NULL,\n"
          "    PRIMARY KEY (account_id)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS account_has_signatory (\n"
          "    account_id character varying(197) NOT NULL REFERENCES account,\n"
          "    public_key bytea NOT NULL REFERENCES signatory,\n"
          "    PRIMARY KEY (account_id, public_key)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS peer (\n"
          "    public_key bytea NOT NULL,\n"
          "    address character varying(21) NOT NULL UNIQUE,\n"
          "    state int NOT NULL DEFAULT 0,\n"
          "    PRIMARY KEY (public_key)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS asset (\n"
          "    asset_id character varying(197),\n"
          "    domain_id character varying(164) NOT NULL REFERENCES domain,\n"
          "    precision int NOT NULL,\n"
          "    data json,\n"
          "    PRIMARY KEY (asset_id)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS account_has_asset (\n"
          "    account_id character varying(197) NOT NULL REFERENCES account,\n"
          "    asset_id character varying(197) NOT NULL REFERENCES asset,\n"
          "    amount bigint NOT NULL,\n"
          "    permissions bit varying NOT NULL,\n"
          "    PRIMARY KEY (account_id, asset_id)\n"
          ");\n"
          "CREATE TABLE IF NOT EXISTS exchange (\n"
          "    asset1_id character varying(197) NOT NULL REFERENCES "
          "asset(asset_id),\n"
          "    asset2_id character varying(197) NOT NULL REFERENCES "
          "asset(asset_id),\n"
          "    asset1 bigint NOT NULL,\n"
          "    asset2 bigint NOT NULL,\n"
          "    PRIMARY KEY (asset1_id, asset2_id)\n"
          ");";
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_STORAGE_IMPL_HPP
