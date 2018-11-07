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

#ifndef IROHA_CLI_ASSERT_CONFIG_HPP
#define IROHA_CLI_ASSERT_CONFIG_HPP

#include <stdexcept>
#include <string>

namespace assert_config {
  /**
   * error message helpers that are used in json validation.
   */
  inline std::string no_member_error(std::string const &member) {
    return "No member '" + member + "'";
  }

  inline std::string type_error(std::string const &value,
                                std::string const &type) {
    return "'" + value + "' is not " + type;
  }

  inline std::string parse_error(std::string const &path) {
    return "Parse error. JSON file path: " + path + "'";
  }

  /**
   * shuts down process when some error occurs.
   * @param error - error message
   */
  inline void fatal_error(std::string const &error) {
    throw std::runtime_error(error);
  }

  /**
   * shuts down process if a given condition is false.
   * @param condition
   * @param error - error message
   */
  inline void assert_fatal(bool condition, std::string const &error) {
    if (!condition) {
      fatal_error(error);
    }
  }
}  // namespace assert_config

#endif  // IROHA_CLI_ASSERT_CONFIG_HPP
