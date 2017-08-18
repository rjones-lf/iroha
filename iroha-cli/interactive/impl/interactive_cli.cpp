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

#include "interactive/interactive_cli.hpp"
#include "interactive/interactive_transaction_cli.hpp"
#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    InteractiveCli::InteractiveCli(std::string account_name) {
      creator_ = account_name;
      menu_points_ = {"1.New transaction (tx)", "2.New query (qry)"};

      main_handler_map_["1"] = &InteractiveCli::startTx;
      main_handler_map_["tx"] = &InteractiveCli::startTx;

      main_handler_map_["2"] = &InteractiveCli::startQuery;
      main_handler_map_["qry"] = &InteractiveCli::startQuery;
    }

    void InteractiveCli::parseMain(std::string line) {
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      // Find in main handler map
      auto command = parser::split(line)[0];

      auto it = main_handler_map_.find(command);
      if (it != main_handler_map_.end()) {
        (this->*it->second)();
      } else {
        std::cout << "Command not found" << std::endl;
      }
    }

    void InteractiveCli::startQuery() {
      InteractiveQueryCli queryCli(creator_);
      queryCli.run();
    }

    void InteractiveCli::startTx() {
      // TODO: implement
    }

    void InteractiveCli::run() {
      std::cout << "Welcome to Iroha-Cli. " << std::endl;
      bool is_parsing = true;
      while (is_parsing) {
        printMenu("Choose what to do:", menu_points_);
        auto line = promtString("> ");
        parseMain(line);
      }
    }

  }  // namespace interactive
}  // namespace iroha_cli
