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

#ifndef IROHA_GENERATOR_H
#define IROHA_GENERATOR_H

#include <crypto/common.h>
#include <crypto/hash.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <string>
#include <vector>

namespace iroha {

/**
 * Current state of random generators.
 */
unsigned int SEED_ = 1337;

const char ALPHABET[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/**
 * returns a number in a range [min, max)
 */
int64_t random_number(int64_t min, int64_t max) {
  return min + (rand_r(&SEED_) % (max - min));
}

uint8_t random_printable_char() { return (uint8_t)random_number(32, 126 + 1); }


std::string random_string(size_t length, std::string alphabet = ALPHABET) {
  assert(alphabet.size() > 0);
  std::string s;
  std::generate_n(std::back_inserter(s), length, [&alphabet]() {
    size_t i = (size_t)random_number(0, alphabet.size());
    return (char)alphabet[i];
  });
  return s;
}

std::vector<uint8_t> random_blob(size_t length) {
  std::vector<uint8_t> v(length);
  std::generate_n(v.begin(), length, std::bind(random_number, 0, 256));
  return v;
}


}

#endif  // IROHA_GENERATOR_H
