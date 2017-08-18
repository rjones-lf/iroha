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

#ifndef IROHA_CLI_INTERACTIVE_CLI_HPP
#define IROHA_CLI_INTERACTIVE_CLI_HPP

#include "interactive/interactive_query_cli.hpp"

namespace iroha_cli {
namespace interactive {

class InteractiveCli {
 public:
  /**
   *
   * @param account_name
   */
  explicit InteractiveCli(std::string account_name);
  /**
   *
   */
  void run();
 private:
  // Main menu points
  std::vector<std::string> menu_points_;
  // Account id of creator
  std::string creator_;

  using MainHandler = void (InteractiveCli::*)();
  std::unordered_map<std::string, MainHandler> main_handler_map_;

  void parseMain(std::string line);

  void startQuery();
  void startTx();

};

}
}

#endif  // IROHA_CLI_INTERACTIVE_CLI_HPP
