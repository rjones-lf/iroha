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

#include <ed25519/ed25519/sha256.h>
#include <ed25519/ed25519/sha512.h>
#include "common/types.hpp"

namespace iroha {

  void sha3_256(uint8_t *output, const uint8_t *input, size_t in_size) {
    sha256(output, input, in_size);
  }

  void sha3_512(uint8_t *output, const uint8_t *input, size_t in_size) {
    sha512(output, input, in_size);
  }

  hash256_t sha3_256(const uint8_t *input, size_t in_size) {
    hash256_t h;
    sha3_256(h.data(), input, in_size);
    return h;
  }

  hash512_t sha3_512(const uint8_t *input, size_t in_size) {
    hash512_t h;
    sha3_512(h.data(), input, in_size);
    return h;
  }

  hash256_t sha3_256(const std::string &msg) {
    hash256_t h;
    sha3_256(
        h.data(), reinterpret_cast<const uint8_t *>(msg.data()), msg.size());
    return h;
  }

  hash512_t sha3_512(const std::string &msg) {
    hash512_t h;
    sha3_512(
        h.data(), reinterpret_cast<const uint8_t *>(msg.data()), msg.size());
    return h;
  }

  hash512_t sha3_512(const std::vector<uint8_t> &msg) {
    hash512_t h;
    sha3_512(h.data(), msg.data(), msg.size());
    return h;
  }

  hash256_t sha3_256(const std::vector<uint8_t> &msg) {
    hash256_t h;
    sha3_256(h.data(), msg.data(), msg.size());
    return h;
  }
}  // namespace iroha
