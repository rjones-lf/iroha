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

#include <soci/soci.h>
#include <boost/optional.hpp>

#include "ametsuchi/impl/postgres_options.hpp"
#include "ametsuchi/key_value_storage.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class FlatFile;

    struct ConnectionContext {
      explicit ConnectionContext(std::unique_ptr<KeyValueStorage> block_store);

      std::unique_ptr<KeyValueStorage> block_store;
    };

    class StorageImpl : public Storage {
     protected:
      static expected::Result<bool, std::string> createDatabaseIfNotExist(
          const std::string &dbname,
          const std::string &options_str_without_dbname);

      static expected::Result<ConnectionContext, std::string> initConnections(
          std::string block_store_dir);

      static expected::Result<std::shared_ptr<soci::connection_pool>,
                              std::string>
      initPostgresConnection(std::string &options_str, size_t pool_size = 10);

     public:
      static expected::Result<std::shared_ptr<StorageImpl>, std::string> create(
          std::string block_store_dir,
          std::string postgres_connection,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory_);

      expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
      createTemporaryWsv() override;

      expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage() override;

      /**
       * Insert block without validation
       * @param blocks - block for insertion
       * @return true if all blocks are inserted
       */
      virtual bool insertBlock(
          const shared_model::interface::Block &block) override;

      /**
       * Insert blocks without validation
       * @param blocks - collection of blocks for insertion
       * @return true if inserted
       */
      virtual bool insertBlocks(
          const std::vector<std::shared_ptr<shared_model::interface::Block>>
              &blocks) override;

      virtual void dropStorage() override;

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      on_commit() override;

     protected:
      StorageImpl(std::string block_store_dir,
                  PostgresOptions postgres_options,
                  std::unique_ptr<KeyValueStorage> block_store,
                  std::shared_ptr<soci::connection_pool> connection,
                  std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                      factory);

      /**
       * Folder with raw blocks
       */
      const std::string block_store_dir_;

      // db info
      const PostgresOptions postgres_options_;

     private:
      std::unique_ptr<KeyValueStorage> block_store_;

      std::shared_ptr<soci::connection_pool> connection_;

      // Allows multiple readers and a single writer
      std::shared_timed_mutex rw_lock_;

      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;

      rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
          notifier_;

      logger::Logger log_;

     protected:
      static const std::string &init_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_STORAGE_IMPL_HPP
