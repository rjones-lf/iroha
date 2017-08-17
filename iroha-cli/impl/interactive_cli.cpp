/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     ://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless re   httpquired by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "interactive_cli.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/converters/pb_query_factory.hpp"

#include "client.hpp"
#include "grpc_response_handler.hpp"
#include "model/generators/query_generator.hpp"
#include "model/generators/transaction_generator.hpp"
#include "parser/parser.hpp"

using namespace std;

namespace iroha_cli {

  void InteractiveCli::welcomeMessage() {
    std::cout << "Welcome to Iroha-Cli" << std::endl;
    std::cout << std::endl;
  }

  void InteractiveCli::startTransaction() {
    cout << "Starting a new transaction ..." << endl;
    current_context_ = InteractiveCli::TX;
    // txMenu();
  }

  void InteractiveCli::startQuery() {
    cout << "Starting a new query ..." << endl;
    current_context_ = InteractiveCli::QUERY;
    queryMenu();
  }

  void InteractiveCli::mainMenu() {
    std::cout << "Choose what to do: " << std::endl;
    // Print main menu:
    auto menu_points = {"1.Query", "2.Transaction with Commands"};
    for_each(menu_points.begin(), menu_points.end(),
             [](auto el) { cout << el << endl; });
    // Assign handlers for main menu:
    main_handler_map_["1"] = &InteractiveCli::startQuery;
    main_handler_map_["query"] = &InteractiveCli::startQuery;

    main_handler_map_["2"] = &InteractiveCli::startTransaction;
    main_handler_map_["transaction"] = &InteractiveCli::startTransaction;
  }

  void InteractiveCli::parseMain(std::string line) {
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    // Find in main handler map
    auto it = main_handler_map_.find(line);
    if (it != main_handler_map_.end()) {
      (this->*it->second)();
    } else {
      cout << "Command not found!" << endl;
    }
  }

  void InteractiveCli::printHelp(std::string command,
                                 std::vector<std::string> parameters) {
    cout << "Run " << command << " with following parameters: " << endl;
    for_each(parameters.begin(), parameters.end(),
             [](auto el) { cout << "  " << el << endl; });
  }

  std::string InteractiveCli::promtString(std::string message) {
    std::string line;
    std::cout << message << ": ";
    std::getline(std::cin, line);
    return line;
  }

  std::shared_ptr<iroha::model::Query> InteractiveCli::parseGetAccount(
      std::string line) {
    iroha::model::generators::QueryGenerator generator;
    auto words = parser::split(line);
    vector<string> notes = {"Creator account ID",
                            "Requested account Id"};

    auto time_stamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    auto counter = 0u;
    string creator;
    string acc_id;

    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      creator = params[0];
      acc_id = params[1];

    } else if (words.size() != notes.size() + 1) {
      printHelp("ga", notes);
      return nullptr;
    } else {
      creator = words[1];
      acc_id = words[2];
    }
    return generator.generateGetAccount(time_stamp, creator, counter,
                                        acc_id);
  }

  std::shared_ptr<iroha::model::Query> InteractiveCli::parseGetAccountAssets(
      std::string line) {
    iroha::model::generators::QueryGenerator generator;
    auto words = parser::split(line);
    vector<string> notes = {"Creator account ID",
                            "Requested account Id", "Requested asset id"};
    auto counter = 0u;
    auto time_stamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());

    string creator;
    string account_id;
    string asset_id;

    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      creator = params[0];
      account_id = params[1];
      asset_id = params[2];

    } else   if (words.size() != notes.size() + 1) {
      printHelp("gaa", notes);
      return nullptr;
    } else {
      creator = words[1];
      account_id = words[2];
      asset_id = words[3];
    }

    return generator.generateGetAccountAssets(
        time_stamp, creator, counter, account_id, asset_id);
  }

  std::shared_ptr<iroha::model::Query>
  InteractiveCli::parseGetAccountTransactions(std::string line) {
    iroha::model::generators::QueryGenerator generator;
    auto words = parser::split(line);
    vector<string> notes = {"Creator account ID",
                            "Requested account Id"};
    string creator;
    string account_id;
    auto time_stamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    auto counter = 0u;

    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      creator = params[0];
      account_id = params[1];
    } else   if (words.size() != notes.size() + 1) {
      printHelp("gat", notes);
      return nullptr;
    } else {
      creator = words[1];
      account_id = words[2];

    }

    return generator.generateGetAccountTransactions(time_stamp, creator,
                                                    counter, account_id);
  }

  std::shared_ptr<iroha::model::Query> InteractiveCli::parseGetSignatories(
      std::string line) {
    iroha::model::generators::QueryGenerator generator;
    auto words = parser::split(line);
    vector<string> notes = {"Creator account ID",
                            "Requested account Id"};

    auto time_stamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    auto counter = 0u;
    string creator;
    string acc_id;

    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      creator = params[0];
      acc_id = params[1];

    } else if (words.size() != notes.size() + 1) {
      printHelp("gs", notes);
      return nullptr;
    } else {
      creator = words[1];
      acc_id = words[2];
    }

    return generator.generateGetSignatories(time_stamp, creator,
                                            counter, acc_id);
  }

  void InteractiveCli::queryMenu() {
    std::cout << " -- " << std::endl;
    // Print main menu:
    auto menu_points = {"1.GetAccount (ga)", "2.GetAccountAssets (gaa)",
                        "3.GetAccountTransactions (gat)",
                        "4.GetSignatories (gs)", "0. Back (b)"};
    for_each(menu_points.begin(), menu_points.end(),
             [](auto el) { cout << el << endl; });
    // Assign handlers for queries
    query_handler_map_["1"] = &InteractiveCli::parseGetAccount;
    query_handler_map_["ga"] = &InteractiveCli::parseGetAccount;

    query_handler_map_["2"] = &InteractiveCli::parseGetAccountAssets;
    query_handler_map_["gaa"] = &InteractiveCli::parseGetAccountAssets;

    query_handler_map_["3"] = &InteractiveCli::parseGetAccountTransactions;
    query_handler_map_["gat"] = &InteractiveCli::parseGetAccountTransactions;

    query_handler_map_["4"] = &InteractiveCli::parseGetSignatories;
    query_handler_map_["gs"] = &InteractiveCli::parseGetSignatories;
  }

  void InteractiveCli::parseQuery(std::string line) {
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    // Find in main handler map
    auto command = parser::split(line)[0];
    if (command == "b" || command == "0") {
      current_context_ = InteractiveCli::NONE;
      return;
    }
    auto it = query_handler_map_.find(command);
    if (it != query_handler_map_.end()) {
      auto res = (this->*it->second)(line);
      if (res) {
        query_ = res;
        current_context_ = InteractiveCli::QUERY_CONT;
        resultMenu();
      }
    } else {
      cout << "Command not found" << endl;
    }
  }

  bool InteractiveCli::parseSaveToFile(std::string line) {
    auto words = parser::split(line);
    vector<string> notes = {"Path to save the file"};
    string path;
    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      path = params[0];

    } else if (words.size() != notes.size() + 1) {
      printHelp("save", notes);
      return false;
    } else {
      path = words[1];
    }

    iroha::model::converters::JsonQueryFactory json_factory;
    auto json_string = json_factory.serialize(query_);
    if (not json_string.has_value()) {
      cout << "Error while forming a json" << endl;
      return false;
    }
    std::ofstream output_file(path);
    output_file << json_string.value();
    cout << "Successfully saved!" << endl;
    return true;
  }

  bool InteractiveCli::parseSendToIroha(std::string line) {
    auto words = parser::split(line);
    vector<string> notes = {"Ip Address of the Iroha server",
                            "Iroha server Port"};
    string path;
    nonstd::optional<int> port;
    if (words.size() == 1) {
      std::vector<string> params;
      for_each(notes.begin(), notes.end(), [this, &params](auto param){
        params.push_back(this->promtString(param));
      });
      path = params[0];
      port = parser::toInt(params[1]);

    } else if (words.size() != notes.size() + 1) {
      printHelp("send", notes);
      return false;
    } else {
      path = words[1];
      port = parser::toInt(words[2]);
    }
    if (not port.has_value()) {
      cout << "Port has wrong format" << endl;
      return false;
    }
    CliClient client(words[1], port.value());
    GrpcResponseHandler response_handler;
    response_handler.handle(client.sendQuery(query_));
    return true;
  }

  void InteractiveCli::resultMenu() {
    std::cout << " " << std::endl;
    std::cout << "Choose what to do next: " << std::endl;
    // Print main menu:
    auto menu_points = {"1.Save localy (save)",
                        "2. Send to Iroha via grpc (send)", "0. Back (b)"};
    for_each(menu_points.begin(), menu_points.end(),
             [](auto el) { cout << el << endl; });
    // Assign handlers for queries
    result_handler_map_["1"] = &InteractiveCli::parseSaveToFile;
    result_handler_map_["save"] = &InteractiveCli::parseSaveToFile;

    result_handler_map_["2"] = &InteractiveCli::parseSendToIroha;
    result_handler_map_["send"] = &InteractiveCli::parseSendToIroha;
  }

  void InteractiveCli::parseResult(std::string line) {
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    // Find in main handler map
    auto command = parser::split(line)[0];
    if (command == "b" || command == "0") {
      current_context_ = InteractiveCli::QUERY;
      return;
    }
    auto it = result_handler_map_.find(command);
    if (it != result_handler_map_.end() && (this->*it->second)(line)) {
      current_context_ = InteractiveCli::NONE;
      cout << "-------" << endl;
      cout << endl;
      mainMenu();
    } else {
      cout << "Command not found or not valid. Try again" << endl;
    }
  }

  void InteractiveCli::run() {
    welcomeMessage();
    mainMenu();
    current_context_ = NONE;
    for (std::string line;
         std::cout << " > " && std::getline(std::cin, line);) {
      // if (not line.empty()) {
      switch (current_context_) {
        case NONE:
          parseMain(line);
          break;
        case TX:
          break;
        case QUERY:
          parseQuery(line);
          break;
        case QUERY_CONT:
          parseResult(line);
          break;
      }
      //}
    }
  }

}  // namespace iroha_cli
