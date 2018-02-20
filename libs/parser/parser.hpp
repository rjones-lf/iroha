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
#include <boost/optional.hpp>
#include <sstream>
#include <string>
#include <vector>

#ifndef IROHA_PARSER_HPP
#define IROHA_PARSER_HPP

namespace parser {

  /**
   * Check if string is actually the integer number
   * @param s
   * @return
   */
  bool isIntNumber(const std::string &s);

  /**
   * Parse the first command in the line
   * @param line string to parse
   * @return nullopt if no command found, string otherwise
   */
  boost::optional<std::string> parseFirstCommand(std::string line);

  /**
   * Split line into words
   * @param line
   * @return vector with words
   */
  std::vector<std::string> split(std::string line);

  template <typename T>
  boost::optional<T> parseValue(std::string word) {
    std::stringstream ss(word);
    if (not isIntNumber(word)) {
      return boost::none;
    }
    T val;
    if (ss >> val) {
      return val;
    } else {
      return boost::none;
    }
  }

}  // namespace parser

#endif  // IROHA_PARSER_HPP
