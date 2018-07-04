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

namespace iroha {
  namespace ametsuchi {
    TemporaryWsvImpl::TemporaryWsvImpl(
        std::unique_ptr<pqxx::lazyconnection> connection,
        std::unique_ptr<pqxx::nontransaction> transaction)
        : connection_(std::move(connection)),
          transaction_(std::move(transaction)),
          wsv_(std::make_unique<PostgresWsvQuery>(*transaction_)),
          executor_(std::make_unique<PostgresWsvCommand>(*transaction_)),
          log_(logger::log("TemporaryWSV")) {
      auto query = std::make_shared<PostgresWsvQuery>(*transaction_);
      auto command = std::make_shared<PostgresWsvCommand>(*transaction_);
      command_executor_ = std::make_shared<CommandExecutor>(query, command);
      command_validator_ = std::make_shared<CommandValidator>(query);
      transaction_->exec("BEGIN;");
    }

    expected::Result<void, validation::CommandNameAndError>
    TemporaryWsvImpl::apply(
        const shared_model::interface::Transaction &tx,
        std::function<expected::Result<void, validation::CommandNameAndError>(
            const shared_model::interface::Transaction &, WsvQuery &)>
            apply_function) {
      const auto &tx_creator = tx.creatorAccountId();
      command_executor_->setCreatorAccountId(tx_creator);
      command_validator_->setCreatorAccountId(tx_creator);
      auto execute_command = [this](auto &command, size_t command_index)
          -> expected::Result<void, validation::CommandNameAndError> {
        // Validate command
        return expected::map_error<validation::CommandNameAndError>(
                   boost::apply_visitor(*command_validator_, command.get()),
                   [command_index](CommandError &error) {
                     return validation::CommandNameAndError{
                         error.command_name,
                         (boost::format("stateful validation error: could "
                                        "not validate "
                                        "command with index %d: %s")
                          % command_index % error.toString())
                             .str()};
                   })
            // Execute commands
            .and_res(expected::map_error<validation::CommandNameAndError>(
                boost::apply_visitor(*command_executor_, command.get()),
                [command_index](CommandError &error) {
                  return validation::CommandNameAndError{
                      error.command_name,
                      (boost::format("stateful validation error: could not "
                                     "execute command with index %d: %s")
                       % command_index % error.toString())
                          .str()};
                }));
      };

      transaction_->exec("SAVEPOINT savepoint_;");

      return apply_function(tx, *wsv_) | [this, &execute_command, &tx]()
                 -> expected::Result<void, validation::CommandNameAndError> {
        // check transaction's commands validness
        const auto &commands = tx.commands();
        validation::CommandNameAndError cmd_name_error;
        for (size_t i = 0; i < commands.size(); ++i) {
          // in case of failed command, rollback and return
          if (not execute_command(commands[i], i)
                      .match(
                          [](expected::Value<void> &) { return true; },
                          [&cmd_name_error](
                              expected::Error<validation::CommandNameAndError>
                                  &error) {
                            cmd_name_error = error.error;
                            return false;
                          })) {
            transaction_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
            return expected::makeError(cmd_name_error);
          }
        }
        // success
        transaction_->exec("RELEASE SAVEPOINT savepoint_;");
        return {};
      };
    }

    TemporaryWsvImpl::~TemporaryWsvImpl() {
      transaction_->exec("ROLLBACK;");
    }
  }  // namespace ametsuchi
}  // namespace iroha
