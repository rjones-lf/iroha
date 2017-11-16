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

#include "ametsuchi/storage.hpp"

#include <cmath>
#include <shared_mutex>

#include <cpp_redis/cpp_redis>
#include <nonstd/optional.hpp>
#include <pqxx/pqxx>
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "logger/logger.hpp"
#include "model/converters/json_block_factory.hpp"

namespace iroha {
  namespace ametsuchi {

    struct ConnectionContext {
      ConnectionContext(std::unique_ptr<FlatFile> block_store,
                        std::unique_ptr<cpp_redis::redis_client> index,
                        std::unique_ptr<pqxx::lazyconnection> pg_lazy,
                        std::unique_ptr<pqxx::nontransaction> pg_nontx)
          : block_store(std::move(block_store)),
            index(std::move(index)),
            pg_lazy(std::move(pg_lazy)),
            pg_nontx(std::move(pg_nontx)) {
      }

      std::unique_ptr<FlatFile> block_store;
      std::unique_ptr<cpp_redis::redis_client> index;
      std::unique_ptr<pqxx::lazyconnection> pg_lazy;
      std::unique_ptr<pqxx::nontransaction> pg_nontx;
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

      virtual bool insertBlock(model::Block block) override;

      virtual void dropStorage() override;

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;

     protected:

      StorageImpl(std::string block_store_dir,
                  std::string redis_host,
                  std::size_t redis_port,
                  std::string postgres_options,
                  std::unique_ptr<FlatFile> block_store,
                  std::unique_ptr<cpp_redis::redis_client> index,
                  std::unique_ptr<pqxx::lazyconnection> wsv_connection,
                  std::unique_ptr<pqxx::nontransaction> wsv_transaction);

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

      std::shared_ptr<WsvQuery> wsv_;

      std::shared_ptr<BlockQuery> blocks_;

      model::converters::JsonBlockFactory serializer_;

      // Allows multiple readers and a single writer
      std::shared_timed_mutex rw_lock_;

      logger::Logger log_;

     protected:
      const std::string init_ = R"(
CREATE TABLE IF NOT EXISTS role (
    role_id character varying(45),
    PRIMARY KEY (role_id)
);
CREATE TABLE IF NOT EXISTS domain (
    domain_id character varying(164),
    default_role character varying(45) NOT NULL REFERENCES role(role_id),
    PRIMARY KEY (domain_id)
);
CREATE TABLE IF NOT EXISTS signatory (
    public_key bytea NOT NULL,
    PRIMARY KEY (public_key)
);
CREATE TABLE IF NOT EXISTS account (
    account_id character varying(197),
    domain_id character varying(164) NOT NULL REFERENCES domain,
    quorum int NOT NULL,
    transaction_count int NOT NULL DEFAULT 0,
    data JSONB,
    PRIMARY KEY (account_id)
);
CREATE TABLE IF NOT EXISTS account_has_signatory (
    account_id character varying(197) NOT NULL REFERENCES account,
    public_key bytea NOT NULL REFERENCES signatory,
    PRIMARY KEY (account_id, public_key)
);
CREATE TABLE IF NOT EXISTS peer (
    public_key bytea NOT NULL,
    address character varying(21) NOT NULL UNIQUE,
    PRIMARY KEY (public_key)
);
CREATE TABLE IF NOT EXISTS asset (
    asset_id character varying(197),
    domain_id character varying(164) NOT NULL REFERENCES domain,
    precision int NOT NULL,
    data json,
    PRIMARY KEY (asset_id)
);
CREATE TABLE IF NOT EXISTS account_has_asset (
    account_id character varying(197) NOT NULL REFERENCES account,
    asset_id character varying(197) NOT NULL REFERENCES asset,
    amount decimal NOT NULL,
    PRIMARY KEY (account_id, asset_id)
);
CREATE TABLE IF NOT EXISTS role_has_permissions (
    role_id character varying(45) NOT NULL REFERENCES role,
    permission_id character varying(45),
    PRIMARY KEY (role_id, permission_id)
);
CREATE TABLE IF NOT EXISTS account_has_roles (
    account_id character varying(197) NOT NULL REFERENCES account,
    role_id character varying(45) NOT NULL REFERENCES role,
    PRIMARY KEY (account_id, role_id)
);
CREATE TABLE IF NOT EXISTS account_has_grantable_permissions (
    permittee_account_id character varying(197) NOT NULL REFERENCES account,
    account_id character varying(197) NOT NULL REFERENCES account,
    permission_id character varying(45),
    PRIMARY KEY (permittee_account_id, account_id, permission_id)
);
)";
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_STORAGE_IMPL_HPP
