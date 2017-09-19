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

#include <algorithm>
#include <iterator>
#include <nonstd/optional.hpp>
#include <sstream>
#include <string>
#include <vector>

#ifndef IROHA_PARSER_HPP
#define IROHA_PARSER_HPP

namespace parser {

  /**
   * Parse the first command in the line
   * @param line string to parse
   * @return nullopt if no command found, string otherwise
   */
  nonstd::optional<std::string> parseFirstCommand(std::string line);

  /**
   * Split line into words
   * @param line
   * @return vector with words
   */
  std::vector<std::string> split(std::string line);

  template <typename T>
  nonstd::optional<T> parseValue(std::string word) {
    std::stringstream ss(word);
    T val;
    if (ss >> val) {
      return val;
    } else {
      return nonstd::nullopt;
    }
  }

}  // namespace parser

#endif  // IROHA_PARSER_HPP
