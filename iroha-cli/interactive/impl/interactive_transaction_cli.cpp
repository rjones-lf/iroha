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
      description_map_ = {
          {ADD_ASSET_QTY, "Add Asset Quantity"},
          {ADD_PEER, "Add Peer to Iroha Network"},
          {ADD_SIGN, "Add Signatory to Account"},
          {ASSIGN_M_KEY, "Assign Master Key to Account"},
          {CREATE_ACC, "Create Account"},
          {CREATE_DOMAIN, "Create Domain"},
          {CREATE_ASSET, "Create Asset"},
          {REMOVE_SIGN, "Remove Signatory"},
          {SET_PERM, "Set Permissions to Account"},
          {SET_QUO, "Set Account Quorum"},
          {SUB_ASSET_QTY, "Subtract  Assets Quantity from Account"},
          {TRAN_ASSET, "Transfer Assets"}};

      const auto acc_id = "Account Id";
      const auto ast_id = "Asset Id";
      const auto dom_id = "Domain Id";
      const auto ammout_a = "Amount to add (integer part)";
      const auto ammout_b = "Amount to add (fractional part)";
      const auto peer_id = "Full address of a peer";
      const auto pub_key = "Public Key";
      const auto acc_name = "Account Name";
      const auto ast_name = "Asset name";
      const auto ast_precision = "Asset precision";
      const auto quorum = "Quorum";

      command_params_ = {
          {ADD_ASSET_QTY, {acc_id, ast_id, ammout_a, ammout_b}},
          {ADD_PEER, {peer_id, pub_key}},
          {ADD_SIGN, {acc_id, pub_key}},
          {ASSIGN_M_KEY, {acc_id, pub_key}},
          {CREATE_ACC, {acc_name, dom_id, pub_key}},
          {CREATE_DOMAIN, {dom_id}},
          {CREATE_ASSET, {ast_name, dom_id, ast_precision}},
          {REMOVE_SIGN, {acc_id, pub_key}},
          {SET_PERM, {}},
          {SET_QUO, {acc_id, quorum}},
          {SUB_ASSET_QTY, {}},
          {TRAN_ASSET,
           {std::string("Src") + acc_id, std::string("Dest") + acc_id, ast_id,
            ammout_a, ammout_b}},

      };

      command_handlers_ = {
          {ADD_ASSET_QTY, &InteractiveTransactionCli::parseAddAssetQuantity},
          {ADD_PEER, &InteractiveTransactionCli::parseAddPeer},
          {ADD_SIGN, &InteractiveTransactionCli::parseAddSignatory},
          {ASSIGN_M_KEY, &InteractiveTransactionCli::parseAssignMasterKey},
          {CREATE_ACC, &InteractiveTransactionCli::parseCreateAccount},
          {CREATE_DOMAIN, &InteractiveTransactionCli::parseCreateDomain},
          {CREATE_ASSET, &InteractiveTransactionCli::parseCreateAsset},
          {REMOVE_SIGN, &InteractiveTransactionCli::parseRemoveSignatory},
          {SET_PERM, &InteractiveTransactionCli::parseSetPermissions},
          {SET_QUO, &InteractiveTransactionCli::parseSetQuorum},
          {SUB_ASSET_QTY,
           &InteractiveTransactionCli::parseSubtractAssetQuantity},
          {TRAN_ASSET, &InteractiveTransactionCli::parseTransferAsset}};

      formMenu(commands_points_, command_handlers_, command_params_,
               description_map_);
      // Add "go back" option
      addBackOption(commands_points_);
    }

    void InteractiveTransactionCli::create_result_menu() {
      // --- Add result menu points ---

      auto result_desciption = getCommonDescriptionMap();
      const auto ADD_CMD = "add";

      result_desciption.insert(
          {ADD_CMD, "Add one more command to the transaction"});
      result_desciption.insert(
          {BACK_CODE, "Go back and start a new transaction"});

      result_params_ = getCommonParamsMap();

      result_params_.insert({ADD_CMD, {}});
      result_params_.insert({BACK_CODE, {}});

      result_handlers_ = {
          {SAVE_CODE, &InteractiveTransactionCli::parseSaveFile},
          {SEND_CODE, &InteractiveTransactionCli::parseSendToIroha},
          {ADD_CMD, &InteractiveTransactionCli::parseAddCommand},
          {BACK_CODE, &InteractiveTransactionCli::parseGoBack}};

      formMenu(result_points_, result_handlers_, result_params_,
               result_desciption);
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
      auto raw_command = parser::parseFirstCommand(line);
      if (not raw_command.has_value()) {
        handleEmptyCommand();
        return false;
      }

      auto command_name = raw_command.value();
      if (isBackOption(command_name)) {
        return false;
      }

      auto res = handleParse<std::shared_ptr<iroha::model::Command>>(
          this, line, command_name, command_handlers_, command_params_);

      if (not res.has_value()) {
        return true;
      }

      commands_.push_back(res.value());
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
      auto command_raw = parser::parseFirstCommand(line);
      if (not command_raw.has_value()) {
        handleEmptyCommand();
        return true;
      }
      auto command_name = command_raw.value();
      // Find in result handler map
      auto res = handleParse<bool>(this, line, command_name, result_handlers_,
                                   result_params_);
      if (not res.has_value()) {
        return true;
      }
      return res.value();
    }

    bool InteractiveTransactionCli::parseSendToIroha(
        std::vector<std::string> params) {
      auto address = parseIrohaPeerParams(params);
      if (not address.has_value()) {
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
      CliClient client(address.value().first, address.value().second);
      GrpcResponseHandler response_handler;
      response_handler.handle(client.sendTx(tx));
      printEnd();
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
      printEnd();
      // Stop parsing
      return false;
    }

    bool InteractiveTransactionCli::parseGoBack(
        std::vector<std::string> params) {
      current_context_ = MAIN;
      // Remove all old commands
      commands_.clear();
      printEnd();
      printMenu("Forming a new transaction. Choose command to add: ",
                commands_points_);
      // Continue parsing
      return true;
    }
    bool InteractiveTransactionCli::parseAddCommand(
        std::vector<std::string> params) {
      current_context_ = MAIN;
      printEnd();
      printMenu("Choose command to add: ", commands_points_);
      // Continue parsing
      return true;
    }

  }  // namespace interactive
}  // namespace iroha_cli
