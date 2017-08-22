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

#ifndef IROHA_CLI_INTERACTIVE_TRANSACTION_CLI_HPP
#define IROHA_CLI_INTERACTIVE_TRANSACTION_CLI_HPP

#include "model/command.hpp"
#include "interactive/interactive_common_cli.hpp"
#include "model/generators/command_generator.hpp"
#include <unordered_map>

namespace iroha_cli {
  namespace interactive {
    class InteractiveTransactionCli {
     public:
      explicit InteractiveTransactionCli(std::string creator_account);
      /**
       * Run interactive query command line
       */
      void run();

     private:
      // Creator account id
      std::string creator_;
      iroha::model::generators::CommandGenerator generator_;
      // Context
      MenuContext current_context_;
      // Commands menu points
      std::vector<std::string> commands_points_;
      // Transaction result points
      std::vector<std::string> result_points_;

      // Shortcut commands
      const std::string ADD_ASSET_QTY = "add_ast_qty";
      const std::string ADD_PEER = "add_peer";
      const std::string ADD_SIGN = "add_sign";
      const std::string ASSIGN_M_KEY = "asn_mk";
      const std::string CREATE_ACC = "crt_acc";
      const std::string CREATE_ASSET = "crt_ast";
      const std::string CREATE_DOMAIN = "crt_dmn";
      const std::string REMOVE_SIGN = "rem_sign";
      const std::string SET_PERM = "set_perm";
      const std::string SET_QUO = "set_qrm";
      const std::string SUB_ASSET_QTY = "sub_ast_qty";
      const std::string TRAN_ASSET = "tran_ast";

      void assign_command_handlers();
      void assign_result_handlers();


      // Commands to be formed
      std::vector<std::shared_ptr<iroha::model::Command>> commands_;

      // ---- Command parsers ----
      bool parseCommand(std::string line);

      using CommandHandler = std::shared_ptr<iroha::model::Command> (
          InteractiveTransactionCli::*)(std::string);
      std::unordered_map<std::string, CommandHandler> command_handlers_;

      std::shared_ptr<iroha::model::Command> parseAddAssetQuantity(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseAddPeer(std::string line);
      std::shared_ptr<iroha::model::Command> parseAddSignatory(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseAssignMasterKey(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseCreateAccount(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseCreateAsset(std::string line);
      std::shared_ptr<iroha::model::Command> parseCreateDomain(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseRemoveSignatory(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseSetPermissions(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseSetQuorum(std::string line);
      std::shared_ptr<iroha::model::Command> parseSubtractAssetQuantity(
          std::string line);
      std::shared_ptr<iroha::model::Command> parseTransferAsset(
          std::string line);

      // ---- Result parsers ------
      bool parseResult(std::string line);
      using ResultHandler =
      bool (InteractiveTransactionCli::*)(std::string);
      std::unordered_map<std::string, ResultHandler> result_handlers_;
      // Result handlers
      bool parseSendToIroha(std::string line);
      bool parseSaveFile(std::string line);
      bool parseGoBack(std::string line);
      bool parseAddCommand(std::string line);
    };
  }  // namespace interactive
}  // namespace iroha_cli

#endif  // IROHA_CLI_INTERACTIVE_TRANSACTION_CLI_HPP
