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

#ifndef IROHA_MUTABLE_STORAGE_IMPL_HPP
#define IROHA_MUTABLE_STORAGE_IMPL_HPP

#include <pqxx/connection>
#include <pqxx/nontransaction>
#include <unordered_map>

#include "ametsuchi/mutable_storage.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace model {
    class CommandExecutorFactory;
  }

  namespace ametsuchi {

    class BlockIndex;
    class WsvCommand;

    class MutableStorageImpl : public MutableStorage {
      friend class StorageImpl;

     public:
      MutableStorageImpl(
          shared_model::interface::types::HashType top_hash,
          std::unique_ptr<pqxx::lazyconnection> connection,
          std::unique_ptr<pqxx::nontransaction> transaction,
          std::shared_ptr<model::CommandExecutorFactory> command_executors);

      bool apply(
          const shared_model::interface::Block &block,
          std::function<bool(const shared_model::interface::Block &,
                             WsvQuery &,
                             const shared_model::interface::types::HashType &)>
              function) override;

      ~MutableStorageImpl() override;

     private:
      shared_model::interface::types::HashType top_hash_;
      // ordered collection is used to enforce block insertion order in
      // StorageImpl::commit
      std::map<uint32_t, std::shared_ptr<shared_model::interface::Block>>
          block_store_;

      std::unique_ptr<pqxx::lazyconnection> connection_;
      std::unique_ptr<pqxx::nontransaction> transaction_;
      std::unique_ptr<WsvQuery> wsv_;
      std::unique_ptr<WsvCommand> executor_;
      std::unique_ptr<BlockIndex> block_index_;
      std::shared_ptr<model::CommandExecutorFactory> command_executors_;

      bool committed;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_MUTABLE_STORAGE_IMPL_HPP
