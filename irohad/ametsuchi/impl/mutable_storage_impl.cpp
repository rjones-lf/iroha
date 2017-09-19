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

#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"

namespace iroha {
  namespace ametsuchi {
    MutableStorageImpl::MutableStorageImpl(
        hash256_t top_hash, std::unique_ptr<cpp_redis::redis_client> index,
        std::unique_ptr<pqxx::lazyconnection> connection,
        std::unique_ptr<pqxx::nontransaction> transaction,
        std::shared_ptr<model::CommandExecutorFactory> command_executors)
        : top_hash_(top_hash),
          index_(std::move(index)),
          connection_(std::move(connection)),
          transaction_(std::move(transaction)),
          wsv_(std::make_unique<PostgresWsvQuery>(*transaction_)),
          executor_(std::make_unique<PostgresWsvCommand>(*transaction_)),
          command_executors_(std::move(command_executors)),
          committed(false) {
      index_->multi();
      transaction_->exec("BEGIN;");
    }

    bool MutableStorageImpl::apply(
        const model::Block &block,
        std::function<bool(const model::Block &, WsvQuery &, const hash256_t &)>
        function) {
      auto execute_command = [this](auto command) {
        return command_executors_->getCommandExecutor(command)->execute(
            *command, *wsv_, *executor_);
      };
      auto execute_transaction = [this, execute_command](auto &transaction) {
        return std::all_of(transaction.commands.begin(),
                           transaction.commands.end(), execute_command);
      };

      transaction_->exec("SAVEPOINT savepoint_;");
      auto result = function(block, *wsv_, top_hash_) &&
          std::all_of(block.transactions.begin(),
                      block.transactions.end(), execute_transaction);

      if (result) {
        block_store_.insert(std::make_pair(block.height, block));
        top_hash_ = block.hash;
        transaction_->exec("RELEASE SAVEPOINT savepoint_;");
      } else {
        transaction_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
      }
      return result;
    }

    MutableStorageImpl::~MutableStorageImpl() {
      if (not committed) {
        index_->discard();
        transaction_->exec("ROLLBACK;");
      }
    }
  }  // namespace ametsuchi
}  // namespace iroha
