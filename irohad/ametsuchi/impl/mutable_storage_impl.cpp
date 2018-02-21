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

#include "ametsuchi/impl/mutable_storage_impl.hpp"

#include <memory>
#include "ametsuchi/impl/postgres_block_index.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "model/execution/command_executor_factory.hpp"
#include "model/sha3_hash.hpp"

// TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
// shared_model https://soramitsu.atlassian.net/browse/IR-881
#include "backend/protobuf/from_old_model.hpp"

namespace iroha {
  namespace ametsuchi {
    MutableStorageImpl::MutableStorageImpl(
        hash256_t top_hash,
        std::unique_ptr<pqxx::lazyconnection> connection,
        std::unique_ptr<pqxx::nontransaction> transaction,
        std::shared_ptr<model::CommandExecutorFactory> command_executors)
        : top_hash_(top_hash),
          connection_(std::move(connection)),
          transaction_(std::move(transaction)),
          wsv_(std::make_unique<PostgresWsvQuery>(*transaction_)),
          executor_(std::make_unique<PostgresWsvCommand>(*transaction_)),
          block_index_(std::make_unique<PostgresBlockIndex>(*transaction_)),
          command_executors_(std::move(command_executors)),
          committed(false),
          log_(logger::log("MutableStorage")) {
      transaction_->exec("BEGIN;");
    }

    bool MutableStorageImpl::apply(
        const wBlock new_block,
        std::function<bool(const wBlock,
                           WsvQuery &,
                           const shared_model::crypto::Hash &)> function) {
      // TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
      // shared_model https://soramitsu.atlassian.net/browse/IR-881
      std::shared_ptr<model::Block> block(new_block->makeOldModel());

      auto execute_transaction = [this](auto &transaction) {
        auto execute_command = [this, &transaction](auto command) {
          auto result =
              command_executors_->getCommandExecutor(command)->execute(
                  *command, *wsv_, *executor_, transaction.creator_account_id);
          return result.match(
              [](expected::Value<void> v) { return true; },
              [&](expected::Error<iroha::model::ExecutionError> e) {
                log_->error(e.error.toString());
                return false;
              });
        };
        return std::all_of(transaction.commands.begin(),
                           transaction.commands.end(),
                           execute_command);
      };

      transaction_->exec("SAVEPOINT savepoint_;");
      // TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
      // shared_model https://soramitsu.atlassian.net/browse/IR-881
      shared_model::crypto::Hash new_hash({top_hash_.begin(), top_hash_.end()});

      auto result = function(new_block, *wsv_, new_hash)
          and std::all_of(block->transactions.begin(),
                          block->transactions.end(),
                          execute_transaction);

      if (result) {
        block_store_.insert(std::make_pair(block->height, *block));
        block_index_->index(*block);

        top_hash_ = block->hash;
        transaction_->exec("RELEASE SAVEPOINT savepoint_;");
      } else {
        transaction_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
      }
      return result;
    }

    MutableStorageImpl::~MutableStorageImpl() {
      if (not committed) {
        transaction_->exec("ROLLBACK;");
      }
    }
  }  // namespace ametsuchi
}  // namespace iroha
