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

    InteractiveQueryCli::InteractiveQueryCli(std::string account_name) {
      creator_ = account_name;
      // Fill menu points for queries
      menu_points_ = {"1.GetAccount (ga)", "2.GetAccountAssets (gaa)",
                      "3.GetAccountTransactions (gat)", "4.GetSignatories (gs)",
                      "0.Back (b)"};

      // Assign handlers for queries
      query_handlers_["1"] = &InteractiveQueryCli::parseGetAccount;
      query_handlers_["ga"] = &InteractiveQueryCli::parseGetAccount;

      query_handlers_["2"] = &InteractiveQueryCli::parseGetAccountAssets;
      query_handlers_["gaa"] = &InteractiveQueryCli::parseGetAccountAssets;

      query_handlers_["3"] = &InteractiveQueryCli::parseGetAccountTransactions;
      query_handlers_["gat"] =
          &InteractiveQueryCli::parseGetAccountTransactions;

      query_handlers_["4"] = &InteractiveQueryCli::parseGetSignatories;
      query_handlers_["gs"] = &InteractiveQueryCli::parseGetSignatories;

      result_points_ = {"1. Save to file as json (save)",
                        "2. Send to Iroha (send)", "0. Back (b)"};

      result_handlers_["1"] = &InteractiveQueryCli::parseSaveFile;
      result_handlers_["save"] = &InteractiveQueryCli::parseSaveFile;

      result_handlers_["2"] = &InteractiveQueryCli::parseSendToIroha;
      result_handlers_["send"] = &InteractiveQueryCli::parseSendToIroha;
    }

    void InteractiveQueryCli::run() {
      std::string line;
      bool is_parsing = true;
      current_context_ = MAIN;
      printMenu("Choose query: ", menu_points_);
      while (is_parsing) {
        line = promtString("> ");
        switch (current_context_) {
          case MAIN:
            is_parsing = parseQuery(line);
            break;
          case RESULT:
            is_parsing = not parseResult(line);
            break;
        }
      }
    }

    bool InteractiveQueryCli::parseQuery(std::string line) {
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in main handler map
      auto command = parser::split(line)[0];
      if (command == "b" || command == "0") {
        return false;
      }
      auto it = query_handlers_.find(command);
      if (it != query_handlers_.end()) {
        auto res = (this->*it->second)(line);
        if (res) {
          query_ = res;
          current_context_ = RESULT;
          printMenu("Query is formed. Choose what to do:", result_points_);
        }
      } else {
        std::cout << "Command not found" << std::endl;
      }
      return true;
    }

    std::shared_ptr<iroha::model::Query> InteractiveQueryCli::parseGetAccount(
        std::string line) {
      iroha::model::generators::QueryGenerator generator;
      std::vector<std::string> notes = {"Requested account Id"};

      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      // TODO: assign counter from Iroha Net
      auto counter = 0u;
      std::cout << "get account parsing" << std::endl;
      auto params = parseParams(line, "ga", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];
      return generator.generateGetAccount(time_stamp, creator_, counter,
                                          account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountAssets(std::string line) {
      iroha::model::generators::QueryGenerator generator;
      auto words = parser::split(line);
      std::vector<std::string> notes = {"Requested account Id",
                                        "Requested asset id"};

      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());

      // TODO: assign counter from Iroha Net
      auto counter = 0u;
      auto params = parseParams(line, "gaa", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];
      auto asset_id = params.value()[1];

      return generator.generateGetAccountAssets(time_stamp, creator_, counter,
                                                account_id, asset_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountTransactions(std::string line) {
      iroha::model::generators::QueryGenerator generator;
      auto words = parser::split(line);
      std::vector<std::string> notes = {"Requested account Id"};
      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      // TODO: assign counter from Iroha Net
      auto counter = 0u;
      auto params = parseParams(line, "gat", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];

      return generator.generateGetAccountTransactions(time_stamp, creator_,
                                                      counter, account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetSignatories(std::string line) {
      iroha::model::generators::QueryGenerator generator;
      auto words = parser::split(line);
      std::vector<std::string> notes = {"Requested account Id"};

      auto time_stamp = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());

      // TODO: assign counter from Iroha Net
      auto counter = 0u;
      auto params = parseParams(line, "gs", notes);
      if (not params.has_value()) {
        return nullptr;
      }
      auto account_id = params.value()[0];

      return generator.generateGetSignatories(time_stamp, creator_, counter,
                                              account_id);
    }

    bool InteractiveQueryCli::parseResult(std::string line) {
      transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in result handler map
      auto command = parser::split(line)[0];
      if (command == "b" || command == "0") {
        current_context_ = MAIN;
        std::cout << "------" << std::endl;
        printMenu("Choose query: ", menu_points_);
        return false;
      }
      auto it = result_handlers_.find(command);
      if (it != result_handlers_.end()) {
        return (this->*it->second)(line);
      } else {
        std::cout << "Command not found." << std::endl;
        return false;
      }
    }

    bool InteractiveQueryCli::parseSendToIroha(std::string line) {
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
      CliClient client(address, port.value());
      GrpcResponseHandler response_handler;
      response_handler.handle(client.sendQuery(query_));
      return true;
    };

    bool InteractiveQueryCli::parseSaveFile(std::string line) {
      std::vector<std::string> notes = {"Path to save query json"};
      auto params = parseParams(line, "save", notes);
      if (not params.has_value()) {
        return false;
      }
      auto path = params.value()[0];
      iroha::model::converters::JsonQueryFactory json_factory;
      auto json_string = json_factory.serialize(query_);
      if (not json_string.has_value()) {
        std::cout << "Error while forming a json" << std::endl;
        return false;
      }
      std::ofstream output_file(path);
      output_file << json_string.value();
      std::cout << "Successfully saved!" << std::endl;
      return true;
    }
  }  // namespace interactive
}  // namespace iroha_cli
