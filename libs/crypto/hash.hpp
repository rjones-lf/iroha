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

extern "C" {
#include <SimpleFIPS202.h>
}

#include <nonstd/optional.hpp>
#include <vector>

namespace iroha {
  namespace crypto {
    inline int sha3_224(unsigned char *output, unsigned char *input,
                        size_t in_size) {
      return SHA3_224(output, input, in_size);
    }

    inline int sha3_256(unsigned char *output, unsigned char *input,
                        size_t in_size) {
      return SHA3_256(output, input, in_size);
    }

    inline int sha3_384(unsigned char *output, unsigned char *input,
                        size_t in_size) {
      return SHA3_384(output, input, in_size);
    }

    inline int sha3_512(unsigned char *output, unsigned char *input,
                        size_t in_size) {
      return SHA3_512(output, input, in_size);
    }

    nonstd::optional<std::vector<uint8_t>> sha3_224(uint8_t *input,
                                                    size_t in_size) {
      std::vector<uint8_t> out(224 / 8);
      int res = SHA3_224(out.data(), input, in_size);
      return !res ? nonstd::optional<std::vector<uint8_t>>(out)
                  : nonstd::nullopt;
    }

    nonstd::optional<std::vector<uint8_t>> sha3_256(const uint8_t *input,
                                                    size_t in_size) {
      std::vector<uint8_t> out(256 / 8);
      int res = SHA3_256(out.data(), input, in_size);
      return !res ? nonstd::optional<std::vector<uint8_t>>(out)
                  : nonstd::nullopt;
    }

    nonstd::optional<std::vector<uint8_t>> sha3_384(uint8_t *input,
                                                    size_t in_size) {
      std::vector<uint8_t> out(384 / 8);
      int res = SHA3_384(out.data(), input, in_size);
      return !res ? nonstd::optional<std::vector<uint8_t>>(out)
                  : nonstd::nullopt;
    }

    nonstd::optional<std::vector<uint8_t>> sha3_512(uint8_t *input,
                                                    size_t in_size) {
      std::vector<uint8_t> out(512 / 8);
      int res = SHA3_512(out.data(), input, in_size);
      return !res ? nonstd::optional<std::vector<uint8_t>>(out)
                  : nonstd::nullopt;
    }

    std::string sha3_256_hex(const std::string &message) {
      const int sha256_size = 256 / 8;
      unsigned char digest[sha256_size];

      SHA3_256(digest, reinterpret_cast<const unsigned char *>(message.c_str()),
               message.size());

      return digest_to_hexdigest(digest, sha256_size);
    }

    std::string sha3_256_hex(const std::vector<uint8_t> &message) {
      const int sha256_size = 256 / 8;
      unsigned char digest[sha256_size];

      SHA3_256(digest, message.data(), message.size());

      return digest_to_hexdigest(digest, sha256_size);
    }

    std::string sha3_512_hex(const std::string &message) {
      const int sha512_size = 512 / 8;
      unsigned char digest[sha512_size];

      SHA3_512(digest, reinterpret_cast<const unsigned char *>(message.c_str()),
               message.size());

      return digest_to_hexdigest(digest, sha512_size);
    }
  }  // namespace crypto
}  // namespace iroha

#endif  // IROHA_HASH_H
