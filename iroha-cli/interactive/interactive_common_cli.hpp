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

#ifndef IROHA_CLI_INTERACTIVE_COMMON_CLI_HPP
#define IROHA_CLI_INTERACTIVE_COMMON_CLI_HPP

#include <algorithm>
#include <iostream>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

namespace iroha_cli {
  namespace interactive {

    enum MenuContext { MAIN, RESULT };

    /**
     *
     * @param command
     * @param parameters
     */
    void printHelp(std::string command, std::vector<std::string> parameters);
    /**
     *
     * @param message
     * @param menu_points
     */
    void printMenu(std::string message, std::vector<std::string> menu_points);

    /**
     * Get string from user
     * @param message Message to ask user
     * @return user's input
     */
    std::string promtString(std::string message);

    nonstd::optional<std::vector<std::string>> parseParams(
        std::string line, std::string command_name,
        std::vector<std::string> notes);

  }  // namespace interactive
}  // namespace iroha_cli

#endif  // IROHA_CLI_INTERACTIVE_COMMON_CLI_HPP
