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
#include <unordered_map>
#include <vector>
#include "client.hpp"
#include "grpc_response_handler.hpp"

namespace iroha_cli {
  namespace interactive {

    /**
     * Parsing menu context, used to identify the context of parsing
     */
    enum MenuContext {
      /**
       * Main menu context, used to print all commands/queries
       */
      MAIN,
      /**
       * Result menu, used for send, save tx/query
       */
      RESULT
    };

    using ParamsDescription = std::vector<std::string>;
    using DesciptionMap = std::unordered_map<std::string, std::string>;
    using MenuPoints = std::vector<std::string>;

    using ParamsMap = std::unordered_map<std::string, ParamsDescription>;

    const std::string SAVE_CODE = "save";
    const std::string SEND_CODE = "send";
    const std::string BACK_CODE = "b";

    DesciptionMap getCommonDescriptionMap();

    ParamsMap getCommonParamsMap();

    void handleEmptyCommand();

    void handleUnknownCommand(std::string command);

    /**
     *
     * @param menu
     */
    void addBackOption(MenuPoints& menu);

    /**
     *
     */
    bool isBackOption(std::string command);

    void printEnd();

    /**
     * Print help for cli command.
     * @param command - name of the cli command
     * @param parameters needed to run the command
     */
    void printHelp(std::string command, std::vector<std::string> parameters);
    /**
     * Pretty Print of menu
     * @param message - message to print before menu
     * @param menu_points - elements of the menu
     */
    void printMenu(std::string message, MenuPoints menu_points);

    /**
     * Get string input from user
     * @param message Message to ask user
     * @return user's input
     */
    std::string promtString(std::string message);

    /**
     * Parse parameters in interactive and shortcuted mode.
     * Function run interactive mode if in line only the command name is passed.
     * Function will parse all needed parameters from line if the line with
     * commands is passed, it will print help if there are not enough parameters
     * in line.
     * @param line - cli line to parse
     * @param command_name - command name to print
     * @param notes - parameters needed to run the command
     * @return vector with needed parameters
     */
    nonstd::optional<std::vector<std::string>> parseParams(
        std::string line, std::string command_name, ParamsMap params_map);

    /**
     * Add menu point to vector menu
     * @param menu_points to add new point
     * @param description of the command to add
     * @param command_short_name command short name
     */
    size_t addMenuPoint(std::vector<std::string>& menu_points,
                        std::string description,
                        std::string command_short_name);

    template <typename K, typename V>
    std::size_t getNextIndex(std::unordered_map<K, V> parsers_map) {
      return parsers_map.size() == 0 ? 1 : parsers_map.size();
    };

    /**
     * Find in unordered map
     * @tparam K key type
     * @tparam V value type
     * @param command_name - key to find in the map
     * @param params_map - map to find
     * @return nullopt if key not found, value if found
     */
    template <typename K, typename V>
    nonstd::optional<V> findInHandlerMap(K command_name,
                                         std::unordered_map<K, V> params_map) {
      auto it = params_map.find(command_name);
      if (it == params_map.end()) {
        // Command not found, report error
        handleUnknownCommand(command_name);
        return nonstd::nullopt;
      }
      return it->second;
    }

    /**
     *
     * @param params
     * @return
     */
    nonstd::optional<std::pair<std::string, int>> parseIrohaPeerParams(
        ParamsDescription params);

    template <typename T, typename V, typename C>
    nonstd::optional<T> handleParse(
        C class_pointer, std::string line, std::string command_name,
        std::unordered_map<std::string, V> parsers_map, ParamsMap params_map) {
      std::cout << "handle parse triggered" << std::endl;
      auto parser = findInHandlerMap(command_name, parsers_map);
      if (not parser.has_value()) {
        std::cout << "Parser for command not found" << std::endl;
        return nonstd::nullopt;
      }
      auto params = parseParams(line, command_name, params_map);
      if (not params.has_value()) {
        std::cout << "Parse params returned no value" << std::endl;
        return nonstd::nullopt;
      }
      return (class_pointer->*parser.value())(params.value());
    };

    /**
     * Add new  cli command to menu points and menu handlers (parsers)
     * @tparam V - type of parser
     * @param command_name - short command name, will be used as id
     * @param command_description - Description of command to add to menu
     * @param menu_points - map of menu points
     * @param params_map - map of used parsers
     * @param parser - function to parse command
     */
    template <typename V>
    void addCliCommand(MenuPoints& menu_points,
                       std::unordered_map<std::string, V>& parsers_map,
                       const std::string command_name,
                       const std::string command_description, V parser) {
      // Add menu point and get the index in menu of current command
      auto index = std::to_string(
          addMenuPoint(menu_points, command_description, command_name));
      // Add parser for this command
      parsers_map[index] = parser;
      parsers_map[command_name] = parser;
    }

    template <typename V>
    void formMenu(MenuPoints& menu_points,
                  std::unordered_map<std::string, V>& parsers_map,
                  ParamsMap& paramsMap, const DesciptionMap desciptionMap) {
      // Add menu point and get the index in menu of current command

      std::for_each(desciptionMap.begin(), desciptionMap.end(),
                    [&parsers_map, &menu_points, &paramsMap](auto val) {
                      auto command_name = val.first;
                      auto command_description = val.second;
                      auto parser = parsers_map.at(command_name);

                      auto index = std::to_string(addMenuPoint(
                          menu_points, command_description, command_name));
                      parsers_map[index] = parser;
                      paramsMap[index] = paramsMap.at(command_name);
                    });
    }

  }  // namespace interactive
}  // namespace iroha_cli

#endif  // IROHA_CLI_INTERACTIVE_COMMON_CLI_HPP
