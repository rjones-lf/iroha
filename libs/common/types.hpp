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

#ifndef IROHA_COMMON_HPP
#define IROHA_COMMON_HPP

#include <array>
#include <crypto/base64.hpp>
#include <cstdio>

/**
 * This file defines common types used in iroha.
 *
 * std::string is convenient to use but it is not safe, because we can not
 * guarantee at compile-time fixed length of the string.
 *
 * For std::array it is possible, so we prefer it over std::string.
 */
namespace iroha {

  using byte_t = uint8_t;

  static const std::string code = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  /**
   * Base type which represents blob of fixed size.
   */
  template <size_t size_>
  class blob_t : public std::array<byte_t, size_> {
    /**
     * Dark magic of C++, do not touch pls :)
     * author: @warchant
     */
   public:
    /**
     * In compile-time returns size of current blob.
     */
    constexpr static size_t size() { return size_; }

    /**
     * Converts current blob to std::string
     */
    std::string to_string() const noexcept {
      return std::string{this->begin(), this->end()};
    }

    /**
     * Converts current blob to base64, represented as std::string
     */
    std::string to_base64() const noexcept {
      return base64_encode(this->data(), size_);
    }

    /**
     * Converts current blob to hex string.
     */
    std::string to_hexstring() const noexcept {
      std::string res(size_ * 2, 0);
      uint8_t front, back;
      auto ptr = this->data();
      for (uint32_t i = 0, k = 0; i < size_; i++) {
        front = (uint8_t)(ptr[i] & 0xF0) >> 4;
        back = (uint8_t)(ptr[i] & 0xF);
        res[k++] = code[front];
        res[k++] = code[back];
      }
      return res;
    }
  };



  template <size_t size>
  using hash_t = blob_t<size>;

  // fixed-size hashes
  using hash224_t = hash_t<224 / 8>;
  using hash256_t = hash_t<256 / 8>;
  using hash384_t = hash_t<384 / 8>;
  using hash512_t = hash_t<512 / 8>;

  namespace ed25519 {
    using sig_t = blob_t<64>;  // ed25519 sig is 64 bytes length
    using pubkey_t = blob_t<32>;
    using privkey_t = blob_t<64>;

    struct keypair_t {
      pubkey_t pubkey;
      privkey_t privkey;
    };
  }

  // timestamps
  using ts64_t = uint64_t;
  using ts32_t = uint32_t;

}  // namespace iroha
#endif  // IROHA_COMMON_HPP
