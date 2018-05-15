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
#include <ciso646>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <boost/optional.hpp>

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

  static const std::string code = {'0',
                                   '1',
                                   '2',
                                   '3',
                                   '4',
                                   '5',
                                   '6',
                                   '7',
                                   '8',
                                   '9',
                                   'a',
                                   'b',
                                   'c',
                                   'd',
                                   'e',
                                   'f'};

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
     * Initialize blob value
     */
    blob_t() {
      this->fill(0);
    }

    /**
     * In compile-time returns size of current blob.
     */
    constexpr static size_t size() {
      return size_;
    }

    /**
     * Converts current blob to std::string
     */
    std::string to_string() const noexcept {
      return std::string{this->begin(), this->end()};
    }

    /**
     * Converts current blob to hex string.
     */
    std::string to_hexstring() const noexcept {
      std::string res(size_ * 2, 0);
      auto ptr = this->data();
      for (uint32_t i = 0, k = 0; i < size_; i++) {
        const auto front = (uint8_t)(ptr[i] & 0xF0) >> 4;
        const auto back = (uint8_t)(ptr[i] & 0xF);
        res[k++] = code[front];
        res[k++] = code[back];
      }
      return res;
    }

    static blob_t<size_> from_string(const std::string &data) {
      if (data.size() != size_) {
        throw BadFormatException("blob_t: input string has incorrect length " + std::to_string(data.size()));
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
   * Bind operator. If argument has value, dereferences argument and calls
   * given function, which should return wrapped value
   * operator| is used since it has to be binary and left-associative
   * Non-void returning specialization
   *
   * boost::optional<int> f();
   * boost::optional<double> g(int);
   *
   * boost::optional<double> d = f()
   *    | g;
   *
   * @tparam T - monadic type
   * @tparam Transform - transform function type
   * @param t - monadic value
   * @param f - function, which takes dereferenced value, and returns
   * wrapped value
   * @return monadic value, which can be of another type
   */
  template <typename T, typename Transform>
  auto operator|(T t, Transform f) ->
      typename std::enable_if<not std::is_same<decltype(f(*t)), void>::value,
                              decltype(f(*t))>::type {
    if (t) {
      return f(*t);
    }
    return {};
  }

  /**
   * Bind operator. If argument has value, dereferences argument and calls
   * given function, which should return wrapped value
   * operator| is used since it has to be binary and left-associative
   * Void specialization
   *
   * boost::optional<int> f();
   * void g(int);
   *
   * f() | g;
   *
   * @tparam T - monadic type
   * @tparam Transform - transform function type
   * @param t - monadic value
   * @param f - function, which takes dereferenced value, and returns
   * wrapped value
   * @return monadic value, which can be of another type
   */
  template <typename T, typename Transform>
  auto operator|(T t, Transform f) -> typename std::
      enable_if<std::is_same<decltype(f(*t)), void>::value>::type {
    if (t) {
      f(*t);
    }
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
    return [&map](auto key) -> boost::optional<typename C::mapped_type> {
      auto it = map.find(key);
      if (it != std::end(map)) {
        return it->second;
      }
      return boost::none;
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
      return boost::make_optional(object);
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
      return boost::make_optional(object);
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
  using privkey_t = blob_t<32>;

  struct keypair_t {
    keypair_t() = default;

    keypair_t(pubkey_t pubkey, privkey_t privkey)
        : pubkey(pubkey), privkey(privkey) {}

    pubkey_t pubkey;
    privkey_t privkey;
  };

  // timestamps
  using ts64_t = uint64_t;
  using ts32_t = uint32_t;

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
