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

#include <unordered_map>
#include "logger/logger.hpp"
#include "model/query.hpp"
#include "model/transaction.hpp"

#ifndef IROHA_INTERACTIVE_CLI_HPP
#define IROHA_INTERACTIVE_CLI_HPP

namespace iroha_cli {
  class InteractiveCli {
   public:
    /**
     * Run interactive command line client
     */
    void run();

   private:
    // Menus
    void welcomeMessage();
    void mainMenu();
    void queryMenu();
    void resultMenu();
    //void txMenu();

    enum MenuContext { NONE, QUERY, QUERY_CONT, TX };
    InteractiveCli::MenuContext current_context_;
    std::shared_ptr<iroha::model::Query> query_;
    iroha::model::Transaction tx_;

    void printHelp(std::string command, std::vector<std::string> parameters);

    std::string promtString(std::string message);
    // Main handlers:
    void startQuery();
    void startTransaction();
    //
    void parseMain(std::string line);
    void parseQuery(std::string line);
    //void parseTx(std::string line);
    //void parseCommand(std::string line);
    void parseResult(std::string line);
    // Query handlers
    std::shared_ptr<iroha::model::Query> parseGetAccount(std::string line);
    std::shared_ptr<iroha::model::Query> parseGetAccountAssets(
        std::string line);
    std::shared_ptr<iroha::model::Query> parseGetAccountTransactions(
        std::string line);
    std::shared_ptr<iroha::model::Query> parseGetSignatories(std::string line);
    // Command handlers

    // Result handlers
    bool parseSaveToFile(std::string line);
    bool parseSendToIroha(std::string line);

    // Cli handler maps:
    using MainHandler = void (InteractiveCli::*)();
    using QueryHandler =
        std::shared_ptr<iroha::model::Query> (InteractiveCli::*)(std::string);
    using ResultHandler = bool (InteractiveCli::*)(std::string);
    std::unordered_map<std::string, MainHandler> main_handler_map_;
    std::unordered_map<std::string, QueryHandler> query_handler_map_;
    std::unordered_map<std::string, ResultHandler> result_handler_map_;
  };
}  // namespace iroha_cli

#endif  // IROHA_INTERACTIVE_CLI_HPP
