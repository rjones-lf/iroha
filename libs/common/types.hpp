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

#ifndef IROHA_COMMON_TYPES_HPP
#define IROHA_COMMON_TYPES_HPP

#include <array>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include "operators.hpp"

#include <nonstd/optional.hpp>

/**
 * This file defines common types used in iroha.
 *
 * std::string is convenient to use but it is not safe, because we can not
 * guarantee at compile-time fixed length of the string.
 *
 * For std::array it is possible, so we prefer it over std::string.
 */

namespace iroha {
  using BadFormatException = std::invalid_argument;
  using byte_t = uint8_t;

  static const std::string code = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  /**
   * Base type which represents blob of fixed size.
   *
   * std::string is convenient to use but it is not safe.
   * We can not specify the fixed length for string.
   *
   * For std::array it is possible, so we prefer it over std::string.
   */
  template <size_t size_>
  class blob_t : public std::array<byte_t, size_> {
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

    static blob_t<size_> from_string(const std::string &data) {
      if (data.size() != size_) {
        throw BadFormatException("blob_t: input string has incorrect length");
      }

      blob_t<size_> b;
      std::copy(data.begin(), data.end(), b.begin());

      return b;
    }
  };

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

  /**
   * Convert string of raw bytes to printable hex string
   * @param str
   * @return
   */
  inline std::string bytestringToHexstring(const std::string &str) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto &c : str) {
      ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
  }

  /**
   * Convert printable hex string to string of raw bytes
   * @param str
   * @return
   */
  inline nonstd::optional<std::string> hexstringToBytestring(
      const std::string &str) {
    if (str.empty() or str.size() % 2 != 0) {
      return nonstd::nullopt;
    }
    std::string result(str.size() / 2, 0);
    for (size_t i = 0; i < result.length(); ++i) {
      std::string byte = str.substr(i * 2, 2);
      try {
        result.at(i) = std::stoul(byte, nullptr, 16);
      } catch (const std::invalid_argument &e) {
        return nonstd::nullopt;
      } catch (const std::out_of_range &e) {
        return nonstd::nullopt;
      }
    }
    return result;
  }

  /**
   * Create map get function for value retrieval by key
   * @tparam K - map key type
   * @tparam V - map value type
   * @param map - map for value retrieval
   * @return function which takes key, returns value if key exists,
   * nullopt otherwise
   */
  template <typename C>
  auto makeOptionalGet(C map) {
    return [&map](auto key) -> nonstd::optional<typename C::mapped_type> {
      auto it = map.find(key);
      if (it != std::end(map)) {
        return it->second;
      }
      return nonstd::nullopt;
    };
  }

  /**
   * Return function which invokes class method by pointer to member with
   * provided arguments
   *
   * class A {
   * int f(int, double);
   * }
   *
   * A a;
   * int i = makeMethodInvoke(a, 1, 1.0);
   *
   * @tparam T - provided class type
   * @tparam Args - provided arguments types
   * @param object - class object
   * @param args - function arguments
   * @return described function
   */
  template <typename T, typename... Args>
  auto makeMethodInvoke(T &object, Args &&... args) {
    return [&](auto f) { return (object.*f)(std::forward<Args>(args)...); };
  }

  /**
   * Assign the value to the object member
   * @tparam V - object member type
   * @tparam B - object type
   * @param object - object value for member assignment
   * @param member - pointer to member in block
   * @return object with deserialized member on success, nullopt otherwise
   */
  template <typename V, typename B>
  auto assignObjectField(B object, V B::*member) {
    return [=](auto value) mutable {
      object.*member = value;
      return nonstd::make_optional(object);
    };
  }

  /**
   * Assign the value to the object member. Block is wrapped in monad
   * @tparam P - monadic type
   * @tparam V - object member type
   * @tparam B - object type
   * @param object - object value for member assignment
   * @param member - pointer to member in object
   * @return object with deserialized member on success, nullopt otherwise
   */
  template <template <typename C> class P, typename V, typename B>
  auto assignObjectField(P<B> object, V B::*member) {
    return [=](auto value) mutable {
      (*object).*member = value;
      return nonstd::make_optional(object);
    };
  }

  template <size_t size>
  using hash_t = blob_t<size>;

  // fixed-size hashes
  using hash224_t = hash_t<224 / 8>;
  using hash256_t = hash_t<256 / 8>;
  using hash384_t = hash_t<384 / 8>;
  using hash512_t = hash_t<512 / 8>;

  using sig_t = blob_t<64>;  // ed25519 sig is 64 bytes length
  using pubkey_t = blob_t<32>;
  using privkey_t = blob_t<64>;

  struct keypair_t {
    pubkey_t pubkey;
    privkey_t privkey;
  };

  // timestamps
  using ts64_t = uint64_t;
  using ts32_t = uint32_t;

  /*
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
        return this->int_part == rhs.int_part && this->frac_part ==
    rhs.frac_part;
      }

      bool operator!=(const Amount &rhs) const {
        return !operator==(rhs);
      }

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
  */

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
#endif  // IROHA_COMMON_TYPES_HPP
