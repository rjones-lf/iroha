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

#include "interactive/interactive_query_cli.hpp"
#include <fstream>
#include "client.hpp"
#include "grpc_response_handler.hpp"
#include "model/converters/json_query_factory.hpp"
#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    void InteractiveQueryCli::create_queries_menu() {
      // -- Fill menu points for queries --
      add_menu_point(menu_points_, "Get Account Information", GET_ACC);
      add_menu_point(menu_points_, "Get Account's Assets", GET_ACC_AST);
      add_menu_point(menu_points_, "Get Account's Transactions", GET_ACC_TX);
      add_menu_point(menu_points_, "Get Account's Signatories", GET_ACC_SIGN);
      // Add "go back" option
      menu_points_.push_back("0.Back (b)");

      // --- Assign handlers for queries ---
      query_handlers_["1"] = &InteractiveQueryCli::parseGetAccount;
      query_handlers_[GET_ACC] = &InteractiveQueryCli::parseGetAccount;

      query_handlers_["2"] = &InteractiveQueryCli::parseGetAccountAssets;
      query_handlers_[GET_ACC_AST] =
          &InteractiveQueryCli::parseGetAccountAssets;

      query_handlers_["3"] = &InteractiveQueryCli::parseGetAccountTransactions;
      query_handlers_[GET_ACC_TX] =
          &InteractiveQueryCli::parseGetAccountTransactions;

      query_handlers_["4"] = &InteractiveQueryCli::parseGetSignatories;
      query_handlers_[GET_ACC_SIGN] = &InteractiveQueryCli::parseGetSignatories;

      // --- Assign query parameters ---
      query_params_[GET_ACC] = {"Requested account Id"};
      query_params_[GET_ACC_AST] = {"Requested account Id",
                                    "Requested asset id"};
      query_params_[GET_ACC_TX] = {"Requested account Id"};
      query_params_[GET_ACC_SIGN] = {"Requested account Id"};
    }

    void InteractiveQueryCli::create_result_menu() {
      add_menu_point(result_points_, "Save to file as json", "save");
      add_menu_point(result_points_, "Send to Iroha Peer","send");
      // Add go back option
      result_points_.push_back("0. Back (b)");

      result_handlers_["1"] = &InteractiveQueryCli::parseSaveFile;
      result_handlers_["save"] = &InteractiveQueryCli::parseSaveFile;

      result_handlers_["2"] = &InteractiveQueryCli::parseSendToIroha;
      result_handlers_["send"] = &InteractiveQueryCli::parseSendToIroha;

    }

    InteractiveQueryCli::InteractiveQueryCli(std::string account_name,
                                             uint64_t query_counter) {
      creator_ = account_name;
      counter_ = query_counter;
      create_queries_menu();
      create_result_menu();
    }

    void InteractiveQueryCli::run() {
      std::string line;
      bool is_parsing = true;
      current_context_ = MAIN;
      printMenu("Choose query: ", menu_points_);
      // Creating a new query, increment local counter
      ++counter_;
      // Init timestamp for a new query
      local_time_ = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());

      while (is_parsing) {
        line = promtString("> ");
        switch (current_context_) {
          case MAIN:
            is_parsing = parseQuery(line);
            break;
          case RESULT:
            is_parsing = parseResult(line);
            break;
        }
      }
    }

    bool InteractiveQueryCli::parseQuery(std::string line) {
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Get command name from the line
      auto command_name = parser::split(line)[0];
      if (command_name == "b" || command_name == "0") {
        // Stop parsing
        return false;
      }

      auto opt_parser = findInHandlerMap(command_name, query_handlers_);
      if (not opt_parser.has_value()){
        std::cout << "Command not found" << std::endl;
        // TODO: add logger, or cover with tests. Discuss with others
        return true;
      }

      // Get query parameters from the user
      auto params = parseParams(line, command_name, query_params_);
      if (not params.has_value()){
        //Not every parameter was initialized
        // Continue parsing
        return true;
      }

      // Parse query
      auto res = (this->*opt_parser.value())(params.value());
      query_ = res;
      current_context_ = RESULT;
      printMenu("Query is formed. Choose what to do:", result_points_);
      // Continue parsing
      return true;
    }

    std::shared_ptr<iroha::model::Query> InteractiveQueryCli::parseGetAccount(
        QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetAccount(local_time_, creator_, counter_,
                                           account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountAssets(QueryParams params) {
      auto account_id = params[0];
      auto asset_id = params[1];
      return generator_.generateGetAccountAssets(
          local_time_, creator_, counter_, account_id, asset_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountTransactions(QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetAccountTransactions(local_time_, creator_,
                                                       counter_, account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetSignatories(QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetSignatories(local_time_, creator_, counter_,
                                               account_id);
    }

    bool InteractiveQueryCli::parseResult(std::string line) {
      transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in result handler map
      auto command_name = parser::split(line)[0];
      if (command_name == "b" || command_name == "0") {
        // Give up the last query and start a new one
        current_context_ = MAIN;
        std::cout << "------" << std::endl;
        printMenu("Choose query: ", menu_points_);
        // Continue parsing
        return true;
      }
      // Find specific parser for the command
      auto opt_parser = findInHandlerMap(command_name, result_handlers_);
      if (not opt_parser.has_value()){
        std::cout << "Command not found" << std::endl;
        // TODO: add logger, or cover with tests. Discuss with others
        return true;
      }
      // Fill up the parameters for query from the user
      auto params = parseParams(line, command_name, query_params_);
      if (not params.has_value()) {
        // Not all params where initialized.
        // Continue parsing
        return true;
      }
      return (this->*opt_parser.value())(params.value());
    }

    bool InteractiveQueryCli::parseSendToIroha(QueryParams params) {
      auto address = params[0];
      auto port = parser::toInt(params[1]);
      if (not port.has_value()) {
        std::cout << "Port has wrong format" << std::endl;
        // Continue parsing
        return true;
      }
      CliClient client(address, port.value());
      GrpcResponseHandler response_handler;
      response_handler.handle(client.sendQuery(query_));
      // Stop parsing
      return false;
    }

    bool InteractiveQueryCli::parseSaveFile(QueryParams params) {
      auto path = params[0];
      iroha::model::converters::JsonQueryFactory json_factory;
      auto json_string = json_factory.serialize(query_);
      if (not json_string.has_value()) {
        std::cout << "Error while forming a json" << std::endl;
        // Continue parsing
        return true;
      }
      std::ofstream output_file(path);
      if (not output_file){
        std::cout << "Cannot create file" << std::endl;
        // Continue parsing
        return true;
      }
      output_file << json_string.value();
      std::cout << "Successfully saved!" << std::endl;
      // Stop parsing
      return false;
    }

  }  // namespace interactive
}  // namespace iroha_cli
