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
#include <string>
#include <typeinfo>
#include <algorithm>

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
   public:
    blob_t() = default;

    blob_t(std::string s) {
      if (s.size() != size_)
        throw std::invalid_argument("input string has wrong size");

      std::copy(s.begin(), s.end(), this->begin());
    }

    blob_t(std::vector<uint8_t> b) {
      if (b.size() != size_)
        throw std::invalid_argument("input string has wrong size");

      std::copy(b.begin(), b.end(), this->begin());
    }

    blob_t &operator=(std::string s) {
      if (s.size() != size_)
        throw std::invalid_argument("input string has wrong size");

      std::copy(s.begin(), s.end(), this->begin());
      return *this;
    }

    bool operator==(std::string s) const {
      auto &&data = this->data();
      if (size_ != s.size()) return false;
      for (auto i = 0u; i < size_; i++)
        if (data[i] != s[i]) return false;
      return true;
    }

    /**
     * Builds blob_t from a string of the same size as blob_t.
     * @param s
     * @return
     */
    static blob_t<size_> from_string(std::string s) {
      if (s.size() != size_)
        throw std::invalid_argument("input string has wrong size");

      blob_t<size_> b;
      std::copy(b.begin(), b.end(), s.begin());

      return b;
    }

    /**
     * Builds blob_t from a variadic string.
     * If blob_t size is N, and string size is S, and N>S, then
     * last N-S items will be filled with zeros.
     * @param s
     */
    static blob_t<size_> from_string_var(std::string s) {
      if (s.size() > size_)
        throw std::invalid_argument("input string is too long");

      if (s.size() < size_) {
        auto gap = size_ - s.size();
        s += std::string(gap, '\0');  // fill the gap with zeros
      }

      blob_t<size_> b;
      std::copy(s.begin(), s.end(), b.begin());

      return b;
    }

    /**
     * Builds blob_t from a hex string. This is case insensitive function.
     * @param hex 04AE07e3 is an example, with or without 0x or 0X
     * @return
     */
    static blob_t<size_> from_hexstring(std::string hex) {
      if (hex.size() >= 2) {
        // if it starts with 0x or 0X, then remove them
        if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
          hex.erase(hex.begin(), hex.begin() + 2);
      }

      if (hex.size() != size_ * 2)
        throw std::invalid_argument("input string has wrong size");

      // transform input hex to lower case
      std::transform(hex.begin(), hex.end(), hex.begin(), ::tolower);

      auto to_number = [](char letter) {
        auto pos = code.find(letter);
        if (pos == std::string::npos)
          throw std::invalid_argument(
              "input contains symbol which is not in hex range");
        return pos;
      };

      blob_t<size_> b;
      for (auto i = 0u, pos = 0u; i < size_ * 2; i += 2) {
        b[pos] += to_number(hex[i]) << 4;   // first letter in pair
        b[pos++] += to_number(hex[i + 1]);  // second letter in pair
      }

      return b;
    }

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

  // hex2bytes
  inline std::vector<uint8_t> hex2bytes(const std::string &hex) {
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < hex.length(); i += 2) {
      std::string byteString = hex.substr(i, 2);
      uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
      bytes.push_back(byte);
    }
    return bytes;
  }

  /**
   * Convert string to blob vector
   * @param source - string for conversion
   * @return vector<blob>
   */
  inline std::vector<uint8_t> stringToBytes(const std::string &source) {
    return std::vector<uint8_t>(source.begin(), source.end());
  }

  /**
   * blob vector to string
   * @param source - vector for conversion
   * @return result string
   */
  inline std::string bytesToString(const std::vector<uint8_t> &source) {
    return std::string(source.begin(), source.end());
  }

  // Deserialize hex string to array
  template <size_t size>
  inline void hexstringToArray(const std::string &string, blob_t<size> &array) {
    auto bytes = hex2bytes(string);
    std::copy(bytes.begin(), bytes.end(), array.begin());
  }

  /**
   * Convert string of raw bytes to printable hex string
   * @param str
   * @return
   */
  inline std::string bytestringToHexstring(std::string str) {
    std::string res(str.size() * 2, 0);
    uint8_t front, back;
    auto ptr = str.data();
    for (uint32_t i = 0, k = 0; i < str.size(); i++) {
      front = (uint8_t)(ptr[i] & 0xF0) >> 4;
      back = (uint8_t)(ptr[i] & 0xF);
      res[k++] = code[front];
      res[k++] = code[back];
    }
    return res;
  }

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

  struct Amount {
    uint64_t int_part;
    uint64_t frac_part;

    Amount(uint64_t integer_part, uint64_t fractional_part) {
      int_part = integer_part;
      frac_part = fractional_part;
    }

    Amount() {
      int_part = 0;
      frac_part = 0;
    }

    uint32_t get_frac_number() { return std::to_string(frac_part).length(); }

    uint64_t get_joint_amount(uint32_t precision) {
      auto coef = ipow(10, precision);
      return int_part * coef + frac_part;
    }

    bool operator==(const Amount &rhs) const {
      return this->int_part == rhs.int_part && this->frac_part == rhs.frac_part;
    }

    bool operator!=(const Amount &rhs) const { return !operator==(rhs); }

   private:
    int ipow(int base, int exp) {
      int result = 1;
      while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
      }

      return result;
    }
  };

  // check the type of the derived class
  template <typename Base, typename T>
  inline bool instanceof (const T *ptr) {
    return typeid(Base) == typeid(*ptr);
  }

  template <typename Base, typename T>
  inline bool instanceof (const T &ptr) {
    return typeid(Base) == typeid(ptr);
  }

}  // namespace iroha
#endif  // IROHA_COMMON_HPP
