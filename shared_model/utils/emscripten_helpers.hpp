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

#ifndef IROHA_EMBIND_HELPERS
#define IROHA_EMBIND_HELPERS

#include <emscripten/val.h>

/**
 * Embind doesn't have any integrated representation of 64 bit types.
 * Because `uint64_t` and have a size more than safe size of JS `Number` type,
 * Embind maintainers can't add safe conversion between them.
 */
static uint64_t convert_from_js_to_uint64(const emscripten::val &input) {
  if (input.typeOf().as<std::string>() != "number")
    throw std::invalid_argument("Cannot pass argument as a Number");

  // Get string with a number by Number.toString() JS method
  // It prevents automatic conversion in val.as treats val as JS String
  std::string str_with_ll_uint = input.call<std::string>("toString");

  return std::stoull(str_with_ll_uint);
}

#endif  // IROHA_EMBIND_HELPERS
