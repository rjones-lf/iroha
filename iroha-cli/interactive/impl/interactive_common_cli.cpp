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
#include "interactive/interactive_common_cli.hpp"
#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    void printHelp(std::string command, std::vector<std::string> parameters) {
      std::cout << "Run " << command
                << " with following parameters: " << std::endl;
      std::for_each(parameters.begin(), parameters.end(),
                    [](auto el) { std::cout << "  " << el << std::endl; });
    };

    void printMenu(std::string message, std::vector<std::string> menu_points) {
      std::cout << message << std::endl;
      std::for_each(menu_points.begin(), menu_points.end(),
                    [](auto el) { std::cout << el << std::endl; });
    };

    std::string promtString(std::string message) {
      std::string line;
      std::cout << message << ": ";
      std::getline(std::cin, line);
      return line;
    }

    nonstd::optional<std::vector<std::string>> parseParams(
        std::string line, std::string command_name,
        std::vector<std::string> notes) {
      auto words = parser::split(line);
      std::cout << "Parse params" << std::endl;
      if (words.size() == 1) {
        // Start interactive mode
        std::cout << "Parse interactive " << std::endl;
        std::vector<std::string> params;
        for_each(notes.begin(), notes.end(), [&params](auto param) {
          params.push_back(promtString(param));
        });
        return params;
      } else if (words.size() != notes.size() + 1) {
        // Not enough parameters passed
        std::cout << "Print help " << std::endl;
        printHelp(command_name, notes);
        return nonstd::nullopt;
      } else {
        // Remove command name
        std::cout << "Just erase one word " << std::endl;
        words.erase(words.begin());
        return words;
      }
    }

    nonstd::optional<std::vector<std::string>> parseSend(std::string line) {
      std::vector<std::string> notes = {"Ip Address of the Iroha server",
                                        "Iroha server Port"};
      nonstd::optional<int> port;
      auto params = parseParams(line, "send", notes);
      return params;
    };

  }  // namespace interactive
}  // namespace iroha_cli
