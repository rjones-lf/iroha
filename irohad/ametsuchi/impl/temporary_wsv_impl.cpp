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

#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"

namespace iroha {
  namespace ametsuchi {
    TemporaryWsvImpl::TemporaryWsvImpl(std::unique_ptr<soci::session> sql,
                                       std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                                       factory)
        : sql_(std::move(sql)),
          wsv_(std::make_shared<PostgresWsvQuery>(*sql_, factory)),
          executor_(std::make_shared<PostgresWsvCommand>(*sql_)),
          command_executor_(std::make_shared<PostgresCommandExecutor>(*sql_)),
          command_validator_(std::make_shared<CommandValidator>(wsv_)),
          log_(logger::log("TemporaryWSV")) {
      *sql_ << "BEGIN";
    }

    expected::Result<void, validation::CommandError> TemporaryWsvImpl::apply(
        const shared_model::interface::Transaction &tx,
        std::function<expected::Result<void, validation::CommandError>(
            const shared_model::interface::Transaction &, WsvQuery &)>
            apply_function) {
      const auto &tx_creator = tx.creatorAccountId();
      command_executor_->setCreatorAccountId(tx_creator);
      command_validator_->setCreatorAccountId(tx_creator);
      auto execute_command =
          [this](auto &command) -> expected::Result<void, CommandError> {
        // Validate command
        return boost::apply_visitor(*command_validator_, command.get())
            // Execute command
            | [this, &command] {
                return boost::apply_visitor(*command_executor_, command.get());
              };
      };

      auto savepoint_wrapper = createSavepoint("savepoint_temp_wsv");

      return apply_function(tx, *wsv_) |
                 [savepoint = std::move(savepoint_wrapper),
                  &execute_command,
                  &tx]() -> expected::Result<void, validation::CommandError> {
        // check transaction's commands validity
        const auto &commands = tx.commands();
        validation::CommandError cmd_error;
        for (size_t i = 0; i < commands.size(); ++i) {
          // in case of failed command, rollback and return
          auto cmd_is_valid =
              execute_command(commands[i])
                  .match([](expected::Value<void> &) { return true; },
                         [i, &cmd_error](expected::Error<CommandError> &error) {
                           cmd_error = {error.error.command_name,
                                        error.error.toString(),
                                        true,
                                        i};
                           return false;
                         });
          if (not cmd_is_valid) {
            return expected::makeError(cmd_error);
          }
        }
        // success
        savepoint->release();
        return {};
      };
    }

    std::unique_ptr<TemporaryWsv::SavepointWrapper>
    TemporaryWsvImpl::createSavepoint(const std::string &name) {
      return std::make_unique<TemporaryWsvImpl::SavepointWrapperImpl>(
          SavepointWrapperImpl(*this, name));
    }

    TemporaryWsvImpl::~TemporaryWsvImpl() {
      *sql_ << "ROLLBACK";
    }

    TemporaryWsvImpl::SavepointWrapperImpl::SavepointWrapperImpl(
        const iroha::ametsuchi::TemporaryWsvImpl &wsv,
        std::string savepoint_name)
        : sql_{wsv.sql_},
          savepoint_name_{std::move(savepoint_name)},
          is_released_{false} {
      *sql_ << "SAVEPOINT " + savepoint_name_ + ";";
    };

    void TemporaryWsvImpl::SavepointWrapperImpl::release() {
      is_released_ = true;
    }

    TemporaryWsvImpl::SavepointWrapperImpl::~SavepointWrapperImpl() {
      if (not is_released_) {
        *sql_ << "ROLLBACK TO SAVEPOINT " + savepoint_name_ + ";";
      } else {
        *sql_ << "RELEASE SAVEPOINT " + savepoint_name_ + ";";
      }
    }

  }  // namespace ametsuchi
}  // namespace iroha
