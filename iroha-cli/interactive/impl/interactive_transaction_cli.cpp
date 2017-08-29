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
#include "grpc_response_handler.hpp"
#include "model/converters/json_common.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/generators/transaction_generator.hpp"

#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    void InteractiveTransactionCli::create_command_menu() {
      // -- Fill menu points for commands --
      add_menu_point(commands_points_, "Add Asset Quantity", ADD_ASSET_QTY);
      add_menu_point(commands_points_, "Add Peer to Iroha Network", ADD_PEER);
      add_menu_point(commands_points_, "Add Signatory to Account", ADD_SIGN);
      add_menu_point(commands_points_, "Assign Master Key to Account",
                     ASSIGN_M_KEY);
      add_menu_point(commands_points_, "Create Account", CREATE_ACC);
      add_menu_point(commands_points_, "Create Domain", CREATE_DOMAIN);
      add_menu_point(commands_points_, "Create Asset", CREATE_ASSET);
      add_menu_point(commands_points_, "Remove Signatory", REMOVE_SIGN);
      add_menu_point(commands_points_, "Set Permissions to Account", SET_PERM);
      add_menu_point(commands_points_, "Set Account Quorum", SET_QUO);
      add_menu_point(commands_points_, "Subtract  Assets Quantity from Account",
                     SUB_ASSET_QTY);
      add_menu_point(commands_points_, "Transfer Assets", TRAN_ASSET);
      // Add "back" option
      commands_points_.push_back("0.Back (b)");

      // --- Assign command menu ---
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

      // --- Assign commands parameters ---
      command_params_[ADD_ASSET_QTY] = {"Account Id to add assets", "Asset id",
                                        "Amount to add (integer part)",
                                        "Amount to add (fractional part)"};

      command_params_[ADD_PEER] = {"Full address of a peer", "Public key"};
      command_params_[ADD_SIGN] = {"Account ID", "Public key to add"};
      command_params_[ASSIGN_M_KEY] = {
          "Account ID", "Public key (must be one of the signatories):"};
      command_params_[CREATE_ACC] = {"Account name", "Domain id", "Public Key"};
      command_params_[CREATE_DOMAIN] = {"Full domain id"};
      command_params_[CREATE_ASSET] = {"Asset name", "Full domain id",
                                       "Asset precision"};
      command_params_[REMOVE_SIGN] = {"Account ID", "Public key to remove"};
      // TODO: implement
      command_params_[SET_PERM] = {};
      command_params_[SET_QUO] = {"Account id", "Quorum"};
      command_params_[SUB_ASSET_QTY] = {};
      command_params_[TRAN_ASSET] = {"Src account id", "Dest account id",
                                     "Asset to transfer",
                                     "Amount to transfer (integer part)",
                                     "Amount to transfer (fractional part)"};

      command_params_["save"] = {"Path to save tx in json format"};
      command_params_["send"] = {"Ip Address of the Iroha server",
                                 "Iroha server Port"};
    }

    void InteractiveTransactionCli::create_result_menu() {
      // --- Add result menu points ---
      add_menu_point(result_points_, "Save to file as json transaction",
                     "save");
      add_menu_point(result_points_, "Send transaction to Iroha", "send");
      add_menu_point(result_points_, "Add command to transaction", "add");
      // Add "back" option
      result_points_.push_back("0. Go Back and start new tx (b)");
      // --- Assign result handlers ---

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
        std::string creator_account, uint64_t tx_counter) {
      creator_ = creator_account;
      tx_counter_ = tx_counter;
      create_command_menu();
      create_result_menu();
    }

    void InteractiveTransactionCli::run() {
      std::string line;
      bool is_parsing = true;
      current_context_ = MAIN;
      printMenu("Forming a new transactions, choose command to add: ",
                commands_points_);
      // Creating a new transaction, increment local tx_counter
      ++tx_counter_;
      while (is_parsing) {
        line = promtString("> ");
        switch (current_context_) {
          case MAIN:
            is_parsing = parseCommand(line);
            break;
          case RESULT:
            is_parsing = parseResult(line);
            break;
        }
      }
    }

    bool InteractiveTransactionCli::parseCommand(std::string line) {
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in main handler map
      auto command_name = parser::split(line)[0];
      if (command_name == "b" || command_name == "0") {
        // Return back to main menu
        // Parsing context is changed
        return false;
      }
      // Find specific parser for this command
      auto opt_parser = findInHandlerMap(command_name, command_handlers_);
      if (not opt_parser.has_value()) {
        std::cout << "Command not found" << std::endl;
        return true;
      }
      // Fill up all needed parameters
      auto params = parseParams(line, command_name, command_params_);
      if (not params.has_value()) {
        // Not all params where initialized.
        // Continue parsing
        return true;
      }

      auto res = (this->*opt_parser.value())(params.value());
      commands_.push_back(res);
      current_context_ = RESULT;
      printMenu("Command is formed. Choose what to do:", result_points_);
      return true;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddAssetQuantity(
        std::vector<std::string> params) {
      auto account_id = params[0];
      auto asset_id = params[1];
      iroha::Amount amount;
      auto val_int = parser::toUint64(params[2]);
      auto val_frac = parser::toUint64(params[3]);
      if (not val_int.has_value() || not val_frac.has_value()) {
        std::cout << "Wrong format for amount" << std::endl;
        return nullptr;
      }
      amount.int_part = val_int.value();
      amount.frac_part = val_frac.value();
      return generator_.generateAddAssetQuantity(account_id, asset_id, amount);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddPeer(std::vector<std::string> params) {
      auto address = params[0];
      auto key = params[1];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);
      return generator_.generateAddPeer(address, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAddSignatory(
        std::vector<std::string> params) {
      auto account_id = params[0];
      auto key = params[1];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);
      return generator_.generateAddSignatory(account_id, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseAssignMasterKey(
        std::vector<std::string> params) {
      auto account_id = params[0];
      auto key = params[1];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);
      return generator_.generateAssignMasterKey(account_id, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateAccount(
        std::vector<std::string> params) {
      auto account_id = params[0];
      auto domain_id = params[1];
      auto key = params[2];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);
      return generator_.generateCreateAccount(account_id, domain_id, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateDomain(
        std::vector<std::string> params) {
      auto domain_id = params[0];
      return generator_.generateCreateDomain(domain_id);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseCreateAsset(
        std::vector<std::string> params) {
      auto asset_name = params[0];
      auto domain_id = params[1];
      auto val = parser::toInt(params[2]);
      if (not val.has_value()) {
        std::cout << "Wrong format for precision" << std::endl;
        return nullptr;
      }
      return generator_.generateCreateAsset(asset_name, domain_id, val.value());
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseRemoveSignatory(
        std::vector<std::string> params) {
      auto account_id = params[0];
      auto key = params[1];
      iroha::ed25519::pubkey_t pubkey;
      iroha::hexstringToArray(key, pubkey);
      return generator_.generateRemoveSignatory(account_id, pubkey);
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSetPermissions(
        std::vector<std::string> params) {
      // TODO: implement when change permission model
      std::cout << "Not implemented" << std::endl;
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSetQuorum(std::vector<std::string> params) {
      auto account_id = params[0];
      auto quorum = parser::toUint64(params[1]);
      if (not quorum.has_value()) {
        std::cout << "Wrong format for quorum" << std::endl;
        return nullptr;
      }
      return generator_.generateSetQuorum(account_id, quorum.value());
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseSubtractAssetQuantity(
        std::vector<std::string> params) {
      // TODO: implement
      std::cout << "Not implemented" << std::endl;
      return nullptr;
    }

    std::shared_ptr<iroha::model::Command>
    InteractiveTransactionCli::parseTransferAsset(
        std::vector<std::string> params) {
      auto src_account_id = params[0];
      auto dest_account_id = params[1];
      auto asset_id = params[2];
      iroha::Amount amount;
      auto val_int = parser::toUint64(params[3]);
      auto val_frac = parser::toUint64(params[4]);
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
      auto command_name = parser::split(line)[0];
      // Find in result handler map
      auto opt_parser = findInHandlerMap(command_name, result_handlers_);
      if (not opt_parser.has_value()) {
        std::cout << "Command not found." << std::endl;
        return true;
      }
      // Fill up all needed parameters
      auto params = parseParams(line, command_name, command_params_);
      if (not params.has_value()) {
        // Continue parsing
        return true;
      }
      return (this->*opt_parser.value())(params.value());
    }

    bool InteractiveTransactionCli::parseSendToIroha(
        std::vector<std::string> params) {
      auto address = params[0];
      auto port = parser::toInt(params[1]);
      if (not port.has_value()) {
        std::cout << "Port has wrong format" << std::endl;
        // Continue parsing
        return true;
      }
      // Forming a transaction
      iroha::model::generators::TransactionGenerator tx_generator_;
      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      auto tx = tx_generator_.generateTransaction(time_stamp, creator_,
                                                  tx_counter_, commands_);
      // TODO: sign tx
      CliClient client(address, port.value());
      GrpcResponseHandler response_handler;
      response_handler.handle(client.sendTx(tx));
      // Stop parsing
      return false;
    }
    bool InteractiveTransactionCli::parseSaveFile(
        std::vector<std::string> params) {
      auto path = params[0];
      std::ofstream output_file(path);
      if (not output_file) {
        std::cout << "Wrong path" << std::endl;
        // Continue parsing
        return true;
      }
      // Forming a transaction
      iroha::model::generators::TransactionGenerator tx_generator_;
      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      auto tx = tx_generator_.generateTransaction(time_stamp, creator_,
                                                  tx_counter_, commands_);
      // TODO: sign tx

      iroha::model::converters::JsonTransactionFactory json_factory;
      auto json_doc = json_factory.serialize(tx);
      auto json_string = iroha::model::converters::jsonToString(json_doc);
      output_file << json_string;
      std::cout << "Successfully saved!" << std::endl;
      // Stop parsing
      return false;
    }

    bool InteractiveTransactionCli::parseGoBack(
        std::vector<std::string> params) {
      current_context_ = MAIN;
      // Remove all old commands
      commands_.clear();
      std::cout << "------" << std::endl;
      printMenu("Forming a new transaction. Choose command to add: ",
                commands_points_);
      // Continue parsing
      return true;
    }
    bool InteractiveTransactionCli::parseAddCommand(
        std::vector<std::string> params) {
      current_context_ = MAIN;
      std::cout << "------" << std::endl;
      printMenu("Choose command to add: ", commands_points_);
      // Continue parsing
      return true;
    }

  }  // namespace interactive
}  // namespace iroha_cli
