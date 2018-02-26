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

#include "ametsuchi/impl/temporary_wsv_impl.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "amount/amount.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "model/execution/command_executor_factory.hpp"

namespace iroha {
  namespace ametsuchi {
    TemporaryWsvImpl::TemporaryWsvImpl(
        std::unique_ptr<pqxx::lazyconnection> connection,
        std::unique_ptr<pqxx::nontransaction> transaction,
        std::shared_ptr<model::CommandExecutorFactory> command_executors)
        : connection_(std::move(connection)),
          transaction_(std::move(transaction)),
          wsv_(std::make_unique<PostgresWsvQuery>(*transaction_)),
          executor_(std::make_unique<PostgresWsvCommand>(*transaction_)),
          command_executors_(std::move(command_executors)),
          log_(logger::log("TemporaryWSV")) {
      transaction_->exec("BEGIN;");
    }

    bool TemporaryWsvImpl::apply(
        const shared_model::interface::Transaction &tx,
        std::function<bool(const shared_model::interface::Transaction &,
                           WsvQuery &)> apply_function) {
      const auto &tx_creator = tx.creatorAccountId();
      auto execute_command = [this, &tx_creator](auto command) {
        auto executor = command_executors_->getCommandExecutor(command);
        if (not executor->validate(*command, *wsv_, tx_creator)) {
          return false;
        }
        auto result =
            executor->execute(*command, *wsv_, *executor_, tx_creator);
        return result.match(
            [](expected::Value<void> &v) { return true; },
            [this](expected::Error<iroha::model::ExecutionError> &e) {
              log_->error(e.error.toString());
              return false;
            });
      };

      transaction_->exec("SAVEPOINT savepoint_;");
      auto commands =
          std::accumulate(tx.commands().begin(),
                          tx.commands().end(),
                          std::vector<std::shared_ptr<model::Command>>{},
                          [](auto &vec, const auto &cmd) {
                            auto curr = std::shared_ptr<model::Command>(cmd->makeOldModel());
                            vec.push_back(curr);
                            return vec;
                          });

      auto result = apply_function(tx, *wsv_)
          and std::all_of(commands.begin(),
                          commands.end(),
                          execute_command);
      if (result) {
        transaction_->exec("RELEASE SAVEPOINT savepoint_;");
      } else {
        transaction_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
      }
      return result;
    }

    TemporaryWsvImpl::~TemporaryWsvImpl() {
      transaction_->exec("ROLLBACK;");
    }
  }  // namespace ametsuchi
}  // namespace iroha
