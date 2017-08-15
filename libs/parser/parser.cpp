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

#include "parser.hpp"

namespace parser {

  std::vector<std::string> split(std::string line) {
    std::istringstream iss(line);
    return {std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>{}};
  }

  nonstd::optional<uint64_t> toUint64(std::string word) {
    try {
      auto val = std::stoull(word);
      return val;
    } catch (const std::exception&) {
      return nonstd::nullopt;
    }
  };

  nonstd::optional<int> toInt(std::string word) {
    try {
      auto val = std::stoi(word);
      return val;
    } catch (const std::exception&) {
      return nonstd::nullopt;
    }
  };

}  // namespace parser
