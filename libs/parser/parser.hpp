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

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <nonstd/optional.hpp>

#ifndef IROHA_PARSER_HPP
#define IROHA_PARSER_HPP

namespace parser {

/**
 * Split line into words
 * @param line
 * @return vector with words
 */
std::vector<std::string> split(std::string line);

/**
 * String to Uint64_t
 * @param word
 * @return nullopt if error
 */
nonstd::optional<uint64_t> toUint64(std::string word);

/**
 * String to Int
 * @param word
 * @return nullopt if error
 */
nonstd::optional<int> toInt(std::string word);


}

#endif //IROHA_PARSER_HPP
