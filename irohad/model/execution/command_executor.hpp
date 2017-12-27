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

#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "logger/logger.hpp"
#include "model/command.hpp"

namespace iroha {
  namespace model {
    /**
     * Encapsulates validation and execution logic for the command
     */
    class CommandExecutor {
     public:
      CommandExecutor();

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
       * @return true, if execution is successful
       */
      virtual bool execute(const Command &command,
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

      logger::Logger log_;
    };

    class AppendRoleExecutor : public CommandExecutor {
     public:
      AppendRoleExecutor();
      bool execute(const Command &command,
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
    };

    class DetachRoleExecutor : public CommandExecutor {
     public:
      DetachRoleExecutor();
      bool execute(const Command &command,
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
    };

    class CreateRoleExecutor : public CommandExecutor {
     public:
      CreateRoleExecutor();
      bool execute(const Command &command,
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
    };

    class GrantPermissionExecutor : public CommandExecutor {
     public:
      GrantPermissionExecutor();
      bool execute(const Command &command,
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
    };

    class RevokePermissionExecutor : public CommandExecutor {
     public:
      RevokePermissionExecutor();
      bool execute(const Command &command,
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
    };

    class AddAssetQuantityExecutor : public CommandExecutor {
     public:
      AddAssetQuantityExecutor();

      bool execute(const Command &command,
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
    };

    class SubtractAssetQuantityExecutor : public CommandExecutor {
     public:
      SubtractAssetQuantityExecutor();

      bool execute(const Command &command,
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
    };

    class AddPeerExecutor : public CommandExecutor {
     public:
      AddPeerExecutor();

      bool execute(const Command &command,
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
    };

    class AddSignatoryExecutor : public CommandExecutor {
     public:
      AddSignatoryExecutor();

      bool execute(const Command &command,
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
    };

    class CreateAccountExecutor : public CommandExecutor {
     public:
      CreateAccountExecutor();

      bool execute(const Command &command,
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
    };

    class CreateAssetExecutor : public CommandExecutor {
     public:
      CreateAssetExecutor();

      bool execute(const Command &command,
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
    };

    class CreateDomainExecutor : public CommandExecutor {
     public:
      CreateDomainExecutor();

      bool execute(const Command &command,
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
    };

    class RemoveSignatoryExecutor : public CommandExecutor {
     public:
      RemoveSignatoryExecutor();

      bool execute(const Command &command,
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
    };

    class SetAccountDetailExecutor : public CommandExecutor {
     public:
      SetAccountDetailExecutor();

      bool execute(const Command &command,
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
    };

    class SetQuorumExecutor : public CommandExecutor {
     public:
      SetQuorumExecutor();

      bool execute(const Command &command,
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
    };

    class TransferAssetExecutor : public CommandExecutor {
     public:
      TransferAssetExecutor();

      bool execute(const Command &command,
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
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_EXECUTOR_HPP
