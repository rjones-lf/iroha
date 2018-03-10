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

#ifndef IROHA_COMMAND_EXECUTOR_HPP
#define IROHA_COMMAND_EXECUTOR_HPP

#include <boost/format.hpp>

#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "common/result.hpp"
#include "model/command.hpp"
#include "model/execution/execution_error.hpp"

namespace iroha {
  namespace model {

    /**
     * ExecutionResult is a return type of all execute functions.
     * If execute is successful, result will not contain anything (void value),
     * because execute does not return any value.
     * If execution is not successful, ExecutionResult will contain Execution
     * error with explanation
     *
     * Result is used because it allows for clear distinction between two states
     * - value and error. If we just returned error, it would be confusing, what
     * value of an error to consider as a successful state of execution
     */
    using ExecutionResult = expected::Result<void, ExecutionError>;

    /**
     * Encapsulates validation and execution logic for the command
     */
    class CommandExecutor {
     public:
      /**
       * Check permissions and perform stateful validation
       * @param command - command to be validated
       * @param queries - world state view query interface
       * @param creator - transaction creators account
       * @return true, if validation is successful
       */
      bool validate(const Command &command,
                    ametsuchi::WsvQuery &queries,
                    const std::string &creator_account_id);

      /**
       * Execute the command on the world state view
       * @param command - command to be executed
       * @param queries - world state view query interface
       * @param commands - world state view command interface
       * @return Result, which will contain error with ExecutionError if execute
       * is not successful or void Value otherwise
       */
      virtual ExecutionResult execute(
          const Command &command,
          ametsuchi::WsvQuery &queries,
          ametsuchi::WsvCommand &commands,
          const std::string &creator_account_id) = 0;

      virtual ~CommandExecutor() = default;

     protected:
      /**
       * Check permission for given creator account
       * @param command - command to be validated
       * @param queries - world state view query interface
       * @param creator - transaction creators account
       * @return true, if validation is successful
       */
      virtual bool hasPermissions(const Command &command,
                                  ametsuchi::WsvQuery &queries,
                                  const std::string &creator_account_id) = 0;

      /**
       * Perform stateful validation for the command
       * @param command - command to be validated
       * @param queries - world state view query interface
       * @return true, if command is valid
       */
      virtual bool isValid(const Command &command,
                           ametsuchi::WsvQuery &queries,
                           const std::string &creator_account_id) = 0;

      /**
       * String name of the command
       * @return name of the command for which executor is invoked
       */
      virtual std::string commandName() const noexcept = 0;

      /**
       * Transforms WSVCommand result to an execution result
       * @param result WSVCommand result, which contatins nothing or an error
       * @return ExecutionResult with command name
       * and an error message from result parameter
       */
      ExecutionResult makeExecutionResult(
          const ametsuchi::WsvCommandResult &result) const noexcept;

      /**
       * Creates Error case result out of a given error message
       * @param error_message what message should be displayed to the user
       * @return ExecutionResult which contains command name
       * and provided error message
       */
      ExecutionResult makeExecutionResult(
          const std::string &error_message) const noexcept;

      /**
       * Return execution result with an error with given message if predicate
       * is false
       * @param condition - boolean value which determines result state
       * @param error_message - string which will be part of the error
       * @return ExecutionResult with empty value if predicate is true, Error
       * with error message otherwise
       */
      ExecutionResult errorIfNot(bool condition,
                                 const std::string &error_message) const
          noexcept;
    };

    class AppendRoleExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class DetachRoleExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class CreateRoleExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class GrantPermissionExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class RevokePermissionExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class AddAssetQuantityExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class SubtractAssetQuantityExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class AddPeerExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class AddSignatoryExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class CreateAccountExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class CreateAssetExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class CreateDomainExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class RemoveSignatoryExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class SetAccountDetailExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class SetQuorumExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

    class TransferAssetExecutor : public CommandExecutor {
     public:
      ExecutionResult execute(const Command &command,
                              ametsuchi::WsvQuery &queries,
                              ametsuchi::WsvCommand &commands,
                              const std::string &creator_account_id) override;

     protected:
      bool hasPermissions(const Command &command,
                          ametsuchi::WsvQuery &queries,
                          const std::string &creator_account_id) override;

      bool isValid(const Command &command,
                   ametsuchi::WsvQuery &queries,
                   const std::string &creator_account_id) override;

      std::string commandName() const noexcept override;
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_EXECUTOR_HPP
