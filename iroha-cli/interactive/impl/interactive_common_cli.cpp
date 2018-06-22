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
#include <utility>

#include "interactive/interactive_common_cli.hpp"
#include "parser/parser.hpp"

namespace iroha_cli {
  namespace interactive {

    DescriptionMap getCommonDescriptionMap() {
      return {
          // commonDescriptionMap
          {SAVE_CODE, "Save as json file"},
          {SEND_CODE, "Send to Iroha peer"}
          // commonDescriptionMap
      };
    }

    ParamsMap getCommonParamsMap(const std::string &default_ip,
                                 int default_port) {
      return {
          // commonParamsMap
          {SAVE_CODE, makeParamsData({"Path to save json file"})},
          {SEND_CODE, {
			  std::make_shared<param_t>(std::make_pair("Peer address", default_ip)),
			  std::make_shared<param_t>(std::make_pair("Peer port", std::to_string(default_port)))
			}
		  }
          // commonParamsMap
      };
    }

	ParamsData makeParamsData(std::vector<std::string> params) {
		ParamsData data;
		std::for_each(params.begin(), params.end(), [&data](auto el) {
			data.push_back(std::make_shared<param_t>(std::make_pair(el, "")));
		});
		return data;
	}

    void handleEmptyCommand() {
      std::cout << "Put not empty command" << std::endl;
    }

    void handleUnknownCommand(std::string &command) {
      std::cout << "Command not found: " << command << std::endl;
    }

    void addBackOption(MenuPoints &menu) {
      menu.push_back("0. Back (" + BACK_CODE + ")");
    }

    bool isBackOption(std::string line) {
      auto command = parser::parseFirstCommand(std::move(line));
      return command
          and (*command == "0" or *command == BACK_CODE);
    }

    void printCommandParameters(std::string &command,
                                ParamsData parameters) {
      std::cout << "Run " << command
                << " with following parameters: " << std::endl;
      std::for_each(parameters.begin(), parameters.end(), [](auto el) {
        std::cout << "  " << std::get<0>(*el) << std::endl;
      });
    }

    void printMenu(const std::string &message, MenuPoints menu_points) {
      std::cout << message << std::endl;
      std::for_each(menu_points.begin(), menu_points.end(), [](auto el) {
        std::cout << el << std::endl;
      });
    }

    boost::optional<std::string> promptString(const std::string &message) {
      std::string line;
      std::cout << message << ": ";
      if (not std::getline(std::cin, line)) {
        // Input is a terminating symbol
        return boost::none;
      }
      return line;
    }

	boost::optional<std::string> promptString(std::pair<std::string, std::string> params) {
		std::string message = std::get<0>(params);
		if (not std::get<1>(params).empty()) {
			message += " (" + std::get<1>(params) + ")";
		}
		return promptString(message);
	}

    void printEnd() {
      std::cout << "--------------------" << std::endl;
    }

    boost::optional<std::pair<std::string, uint16_t>> parseIrohaPeerParams(
        ParamsDescription params,
        const std::string &default_ip,
        int default_port) {
      const auto &address = params[0].empty() ? default_ip : params[0];
      auto port = params[1].empty() ? default_port
                                    : parser::parseValue<uint16_t>(params[1]);
      if (not params.empty() and not port) {
        std::cout << "Port has wrong format" << std::endl;
        // Continue parsing
        return boost::none;
      }
      return std::make_pair(address, *port);
    }

    boost::optional<std::vector<std::string>> parseParams(
        std::string line, std::string command_name, ParamsMap params_map) {
      auto params_data =
          findInHandlerMap(command_name, params_map);
      if (not params_data) {
        // Report no params where found for this command
        std::cout << "Command params not found" << std::endl;
        // Stop parsing, something is not implemented
        return boost::none;
      }
      auto words = parser::split(std::move(line));
      if (words.size() == 1) {
        // Start interactive mode
        std::vector<std::string> params;
        std::for_each(params_data.value().begin(),
                      params_data.value().end(),
                      [&params](auto param) {
                        auto val = promptString(*param);
						if (val) {
							if (not val.value().empty()) {
								std::get<1>(*param) = val.value();
								params.push_back(val.value());
							} else if (not std::get<1>(*param).empty()) {
								params.push_back(std::get<1>(*param));
							}
						}
                      });
        if (params.size() != params_data.value().size()) {
          // Wrong params passed
          return boost::none;
        }
        return params;
      } else if (words.size() != params_data.value().size() + 1) {
        // Not enough parameters passed
        printCommandParameters(command_name, params_data.value());
        return boost::none;
      } else {
        // Remove command name, return parameters
        words.erase(words.begin());
        return words;
      }
    }

    size_t addMenuPoint(std::vector<std::string> &menu_points,
                        const std::string &description,
                        const std::string &command_short_name) {
      menu_points.push_back(std::to_string(menu_points.size() + 1) + ". "
                            + description + " (" + command_short_name + ")");
      return menu_points.size();
    }
  }  // namespace interactive
}  // namespace iroha_cli
