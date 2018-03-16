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
#include <boost/optional.hpp>
#include <pqxx/pqxx>
#include <shared_mutex>
#include "logger/logger.hpp"
#include "model/converters/json_block_factory.hpp"

namespace iroha {
  namespace ametsuchi {

    class FlatFile;

    struct ConnectionContext {
      ConnectionContext(std::unique_ptr<FlatFile> block_store,
                        std::unique_ptr<pqxx::lazyconnection> pg_lazy,
                        std::unique_ptr<pqxx::nontransaction> pg_nontx);

      std::unique_ptr<FlatFile> block_store;
      std::unique_ptr<pqxx::lazyconnection> pg_lazy;
      std::unique_ptr<pqxx::nontransaction> pg_nontx;
    };

    class StorageImpl : public Storage {
     protected:
      static expected::Result<ConnectionContext, std::string> initConnections(
          std::string block_store_dir, std::string postgres_options);

     public:
      static expected::Result<std::shared_ptr<StorageImpl>, std::string> create(
          std::string block_store_dir, std::string postgres_connection);

      expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
      createTemporaryWsv() override;

      expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage() override;

      /**
       * Insert block without validation
       * @param blocks - block for insertion
       * @return true if all blocks are inserted
       */
      virtual bool insertBlock(const shared_model::interface::Block &block) override;

      /**
       * Insert blocks without validation
       * @param blocks - collection of blocks for insertion
       * @return true if inserted
       */
      virtual bool insertBlocks(
          const std::vector<std::shared_ptr<shared_model::interface::Block>> &blocks) override;

      virtual void dropStorage() override;

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;

      ~StorageImpl() override;

     protected:
      StorageImpl(std::string block_store_dir,
                  std::string postgres_options,
                  std::unique_ptr<FlatFile> block_store,
                  std::unique_ptr<pqxx::lazyconnection> wsv_connection,
                  std::unique_ptr<pqxx::nontransaction> wsv_transaction);

      /**
       * Folder with raw blocks
       */
      const std::string block_store_dir_;

      // db info
      const std::string postgres_options_;

     private:
      std::unique_ptr<FlatFile> block_store_;

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

#endif  // IROHA_STORAGE_IMPL_HPP
