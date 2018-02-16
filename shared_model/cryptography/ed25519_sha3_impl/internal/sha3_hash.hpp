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

#ifndef IROHA_HASH_H
#define IROHA_HASH_H

#include "common/types.hpp"

namespace iroha {

  void sha3_256(uint8_t *output, const uint8_t *input, size_t in_size);
  void sha3_512(uint8_t *output, const uint8_t *input, size_t in_size);

  hash256_t sha3_256(const uint8_t *input, size_t in_size);
  hash256_t sha3_256(const std::string &msg);
  hash256_t sha3_256(const std::vector<uint8_t> &msg);
  hash512_t sha3_512(const uint8_t *input, size_t in_size);
  hash512_t sha3_512(const std::string &msg);
  hash512_t sha3_512(const std::vector<uint8_t> &msg);
}  // namespace iroha

#endif  // IROHA_HASH_H
