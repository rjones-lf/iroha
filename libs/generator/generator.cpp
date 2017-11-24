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

#include "generator/generator.hpp"
#include <random>

namespace generator {

  int64_t random_number(int64_t min, int64_t max) {
    static std::random_device seed_gen;
    static std::default_random_engine engine(seed_gen());
    std::uniform_int_distribution<> dist(min, max);
    return dist(engine);
  }

  char random_printable_char() {
    return (uint8_t)random_number(32, 126);
  }

  char random_lower_char() { return (char)random_number('a', 'z'); }

  std::string random_string(size_t length,
                            const std::function<char(void)> &generator) {
    std::string s;
    std::generate_n(std::back_inserter(s), length, generator);
    return s;
  }
}  // namespace generator
