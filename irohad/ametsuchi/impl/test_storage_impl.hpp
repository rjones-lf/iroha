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

#ifndef IROHA_TEST_STORAGE_IMPL_HPP
#define IROHA_TEST_STORAGE_IMPL_HPP

#include "ametsuchi/test_storage.hpp"
#include "ametsuchi/impl/storage_impl.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class TestStorageImpl : public TestStorage, public StorageImpl {
     public:

      static std::shared_ptr<TestStorageImpl> create(
          std::string block_store_dir, std::string redis_host,
          std::size_t redis_port, std::string postgres_connection);

      bool insertBlock(model::Block block) override;

      void dropStorage() override;

     protected:
      TestStorageImpl(std::string block_store_dir,
                      std::string redis_host,
                      size_t redis_port,
                      std::string postgres_options,
                      std::unique_ptr<FlatFile> block_store,
                      std::unique_ptr<cpp_redis::redis_client> index,
                      std::unique_ptr<pqxx::lazyconnection> wsv_connection,
                      std::unique_ptr<pqxx::nontransaction> wsv_transaction);
     private:
      logger::Logger log_;

      // storage interface
     public:
      std::unique_ptr<TemporaryWsv> createTemporaryWsv() override;

      std::unique_ptr<MutableStorage> createMutableStorage() override;

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif //IROHA_TEST_STORAGE_IMPL_HPP
