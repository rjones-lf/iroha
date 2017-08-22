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

#include "interactive/interactive_transaction_cli.hpp"
#include <fstream>
#include "client.hpp"
#include "common/types.hpp"
#include "grpc_response_handler.hpp"
#include "interactive/interactive_common_cli.hpp"
#include "model/converters/json_common.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/generators/transaction_generator.hpp"

#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    void InteractiveTransactionCli::assign_command_handlers() {
      // Fill menu points for commands
      commands_points_ = {
          "1.Add Asset Quantity (" + ADD_ASSET_QTY + ")",
          "2.Add Peer to Iroha Network (" + ADD_PEER + ")",
          "3.Add Signatory to Account (" + ADD_SIGN + ")",
          "4.Assign Master Key to Account (" + ASSIGN_M_KEY + ")",
          "5.Create Account (" + CREATE_ACC + ")",
          "6.Create Domain (" + CREATE_DOMAIN + ")",
          "7.Create Asset (" + CREATE_ASSET + ")",
          "8.Remove Signatory (" + REMOVE_SIGN + ")",
          "9.Set Permissions to Account (" + SET_PERM + ")",
          "10.Set Account Quorum (" + SET_QUO + ")",
          "11.Subtract  Assets Quantity from Account (" + SUB_ASSET_QTY + ")",
          "12.Transfer Assets (" + TRAN_ASSET + ")",
          "0.Back (b)"};

      command_handlers_["1"] =
          &InteractiveTransactionCli::parseAddAssetQuantity;
      command_handlers_[ADD_ASSET_QTY] =
          &InteractiveTransactionCli::parseAddAssetQuantity;

      command_handlers_["2"] = &InteractiveTransactionCli::parseAddPeer;
      command_handlers_[ADD_PEER] = &InteractiveTransactionCli::parseAddPeer;

      command_handlers_["3"] = &InteractiveTransactionCli::parseAddSignatory;
      command_handlers_[ADD_SIGN] =
          &InteractiveTransactionCli::parseAddSignatory;

      command_handlers_["4"] = &InteractiveTransactionCli::parseAssignMasterKey;
      command_handlers_[ASSIGN_M_KEY] =
          &InteractiveTransactionCli::parseAssignMasterKey;

      command_handlers_["5"] = &InteractiveTransactionCli::parseCreateAccount;
      command_handlers_[CREATE_ACC] =
          &InteractiveTransactionCli::parseCreateAccount;

      command_handlers_["6"] = &InteractiveTransactionCli::parseCreateDomain;
      command_handlers_[CREATE_DOMAIN] =
          &InteractiveTransactionCli::parseCreateDomain;

      command_handlers_["7"] = &InteractiveTransactionCli::parseCreateAsset;
      command_handlers_[CREATE_ASSET] =
          &InteractiveTransactionCli::parseCreateAsset;

      command_handlers_["8"] = &InteractiveTransactionCli::parseRemoveSignatory;
      command_handlers_[REMOVE_SIGN] =
          &InteractiveTransactionCli::parseRemoveSignatory;

      command_handlers_["9"] = &InteractiveTransactionCli::parseSetPermissions;
      command_handlers_[SET_PERM] =
          &InteractiveTransactionCli::parseSetPermissions;

      command_handlers_["10"] = &InteractiveTransactionCli::parseSetQuorum;
      command_handlers_[SET_QUO] = &InteractiveTransactionCli::parseSetQuorum;

      command_handlers_["11"] =
          &InteractiveTransactionCli::parseSubtractAssetQuantity;
      command_handlers_[SUB_ASSET_QTY] =
          &InteractiveTransactionCli::parseSubtractAssetQuantity;

      command_handlers_["12"] = &InteractiveTransactionCli::parseTransferAsset;
      command_handlers_[TRAN_ASSET] =
          &InteractiveTransactionCli::parseTransferAsset;

    }

    void InteractiveTransactionCli::assign_result_handlers() {
      result_points_ = {"1. Save to file as json (save)",
                        "2. Send to Iroha (send)",
                        "3. Add command (add)"
                        "0. Go Back and start new tx (b)"};

      result_handlers_["1"] = &InteractiveTransactionCli::parseSaveFile;
      result_handlers_["save"] = &InteractiveTransactionCli::parseSaveFile;

      result_handlers_["2"] = &InteractiveTransactionCli::parseSendToIroha;
      result_handlers_["send"] = &InteractiveTransactionCli::parseSendToIroha;

      result_handlers_["3"] = &InteractiveTransactionCli::parseAddCommand;
      result_handlers_["add"] = &InteractiveTransactionCli::parseAddCommand;

      result_handlers_["0"] = &InteractiveTransactionCli::parseGoBack;
      result_handlers_["b"] = &InteractiveTransactionCli::parseGoBack;

    }

    InteractiveTransactionCli::InteractiveTransactionCli(
        std::string creator_account) {
      creator_ = creator_account;
      assign_command_handlers();
      assign_result_handlers();
    }

    void InteractiveTransactionCli::run() {
      std::string line;
      bool is_parsing = true;
      current_context_ = MAIN;
      printMenu("Forming a new transactions, choose command to add: ",
                commands_points_);
      while (is_parsing) {
        line = promtString("> ");
        switch (current_context_) {
          case MAIN:
            is_parsing = parseCommand(line);
            break;
          case RESULT:
            is_parsing = not parseResult(line);
            break;
        }
      }
    }

    bool InteractiveTransactionCli::parseCommand(std::string line) {
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in main handler map
      auto command = parser::split(line)[0];
      if (command == "b" || command == "0") {
        // Return back to main menu
        // Stop parsing
        return false;
      }
      auto it = command_handlers_.find(command);
      if (it != command_handlers_.end()) {
        auto res = (this->*it->second)(line);
        if (res) {
          commands_.push_back(res);
          current_context_ = RESULT;
          printMenu("Command is formed. Choose what to do:", result_points_);
        }
      } else {
        std::cout << "Command not found" << std::endl;
      }
      // Continue parsing
      return true;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddAssetQuantity(std::string line) {
      std::vector<std::string> notes = {"Account Id to add assets", "Asset id",
                                        "Amount to add (integer part)",
                                        "Amount to add (fractional part)"};

      auto params = parseParams(line, "aaq", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];
      auto asset_id = params.value()[1];
      iroha::Amount amount;
      auto val_int = parser::toUint64(params.value()[2]);
      auto val_frac = parser::toUint64(params.value()[3]);
      if (not val_int.has_value() || not val_frac.has_value()) {
        std::cout << "Wrong format for amount" << std::endl;
        return nullptr;
      }
      amount.int_part = val_int.value();
      amount.frac_part = val_frac.value();
      return generator_.generateAddAssetQuantity(account_id, asset_id, amount);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddPeer(std::string line) {
      std::vector<std::string> notes = {"Full address of a peer", "Public key"};
      std::cout << "Not implemented " << std::endl;
      // TODO: disccuss how to make this
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddSignatory(std::string line) {
      std::cout << "Not implemented " << std::endl;
      // TODO: disccuss how to make this
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAssignMasterKey(std::string line) {
      std::cout << "Not implemented " << std::endl;
      // TODO: disccuss how to make this
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateAccount(std::string line) {
      std::vector<std::string> notes = {"Account name", "Domain id",
                                        "Public Key"};
      auto params = parseParams(line, "cac", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];
      auto domain_id = params.value()[1];
      auto key = params.value()[2];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);

      return generator_.generateCreateAccount(account_id, domain_id, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateDomain(std::string line) {
      std::vector<std::string> notes = {"Full domain id"};
      auto params = parseParams(line, "cdo", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto domain_id = params.value()[0];
      return generator_.generateCreateDomain(domain_id);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateAsset(std::string line) {
      std::vector<std::string> notes = {"Full domain id"};
      auto params = parseParams(line, "cas", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto asset_name = params.value()[0];
      auto domain_id = params.value()[1];
      auto val = parser::toInt(params.value()[2]);
      if (not val.has_value()) {
        // error
        return nullptr;
      }
      return generator_.generateCreateAsset(asset_name, domain_id, val.value());
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseRemoveSignatory(std::string line) {
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSetPermissions(std::string line) {
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSetQuorum(std::string line) {
      std::vector<std::string> notes = {"Account id", "Quorum"};
      auto params = parseParams(line, "squ", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];
      auto quorum = parser::toUint64(params.value()[2]);
      if (not quorum.has_value()) {
        std::cout << "Wrong format for quorum" << std::endl;
        return nullptr;
      }
      return generator_.generateSetQuorum(account_id, quorum.value());
    }
    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSubtractAssetQuantity(std::string line) {
      std::cout << "Not implemented" << std::endl;
      // TODO: implement
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseTransferAsset(std::string line) {
      std::vector<std::string> notes = {"Src account id", "Dest account id",
                                        "Asset to transfer",
                                        "Amount to transfer (integer part)",
                                        "Amount to transfer (fractional part)"};

      auto params = parseParams(line, "tas", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto src_account_id = params.value()[0];
      auto dest_account_id = params.value()[1];
      auto asset_id = params.value()[2];
      iroha::Amount amount;
      auto val_int = parser::toUint64(params.value()[3]);
      auto val_frac = parser::toUint64(params.value()[4]);
      if (not val_int.has_value() || not val_frac.has_value()) {
        std::cout << "Wrong format for amount" << std::endl;
        return nullptr;
      }
      amount.int_part = val_int.value();
      amount.frac_part = val_frac.value();
      return generator_.generateTransferAsset(src_account_id, dest_account_id,
                                              asset_id, amount);
    }

    // --------- Result parsers -------------

    bool InteractiveTransactionCli::parseResult(std::string line) {
      transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in result handler map
      auto command = parser::split(line)[0];
      auto it = result_handlers_.find(command);
      if (it != result_handlers_.end()) {
        return (this->*it->second)(line);
      } else {
        std::cout << "Command not found." << std::endl;
        return false;
      }
    }

    bool InteractiveTransactionCli::parseSendToIroha(std::string line) {
      std::vector<std::string> notes = {"Ip Address of the Iroha server",
                                        "Iroha server Port"};
      auto params = parseParams(line, "send", notes);
      if (not params.has_value()) {
        return false;
      }
      auto address = params.value()[0];
      auto port = parser::toInt(params.value()[1]);
      if (not port.has_value()) {
        std::cout << "Port has wrong format" << std::endl;
        return false;
      }
      // Forming a transaction
      iroha::model::generators::TransactionGenerator tx_generator_;
      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      // TODO: assign counter from Iroha Net
      auto counter = 0u;

      auto tx = tx_generator_.generateTransaction(time_stamp, creator_, counter,
                                                  commands_);
      // TODO: sign tx

      CliClient client(address, port.value());
      GrpcResponseHandler response_handler;
      response_handler.handle(client.sendTx(tx));
      return true;
    }
    bool InteractiveTransactionCli::parseSaveFile(std::string line) {
      std::vector<std::string> notes = {"Path to save query json"};
      auto params = parseParams(line, "save", notes);
      if (not params.has_value()) {
        return false;
      }
      auto path = params.value()[0];

      std::ofstream output_file(path);
      if (not output_file) {
        std::cout << "Wrong path" << std::endl;
        return false;
      }
      // Forming a transaction
      iroha::model::generators::TransactionGenerator tx_generator_;
      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      // TODO: assign counter from Iroha Net
      auto counter = 0u;

      auto tx = tx_generator_.generateTransaction(time_stamp, creator_, counter,
                                                  commands_);
      // TODO: sign tx

      iroha::model::converters::JsonTransactionFactory json_factory;
      // TODO: change to other implemenetation
      auto json_doc = json_factory.serialize(tx);
      auto json_string = iroha::model::converters::jsonToString(json_doc);
      output_file << json_string;
      std::cout << "Successfully saved!" << std::endl;
      return true;
    }

    bool InteractiveTransactionCli::parseGoBack(std::string line) {
      current_context_ = MAIN;
      // Remove all old commands
      commands_.clear();
      std::cout << "------" << std::endl;
      printMenu("Forming a new transaction. Choose command to add: ",
                commands_points_);
      return false;
    }
    bool InteractiveTransactionCli::parseAddCommand(std::string line) {
      current_context_ = MAIN;
      std::cout << "------" << std::endl;
      printMenu("Choose command to add: ", commands_points_);
      return false;
    }

  }  // namespace interactive
}  // namespace iroha_cli
