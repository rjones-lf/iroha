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

#ifndef IROHA_PRIVATE_KEY_HPP_
#define IROHA_PRIVATE_KEY_HPP_

#include <array>
#include <common/types.hpp>
#include <cstring>
#include <secure/prng.hpp>
#include <secure/util.hpp>

namespace iroha {
  namespace crypto {

    /**
     * @class PrivateKey
     * @brief Represents secret data, intended for use as private keys in
     * cryptographic algorithms.
     */
    class PrivateKey final {
      std::vector<uint8_t> blob;

     public:
      PrivateKey() = default;                       ///< default constructor
      PrivateKey(PrivateKey const&) = default;      ///< copy constructor
      PrivateKey(PrivateKey&&) noexcept = default;  ///< move constructor
      PrivateKey& operator=(PrivateKey const&) =
          default;  ///< copy assignment operator
      PrivateKey& operator=(PrivateKey&&) noexcept =
          default;  ///< move assignment operator
      PrivateKey

      /**
       * @brief Destructor. Securely erase private key data.
       */
      ~PrivateKey() {
        secure::erase(reinterpret_cast<void*>(blob.data()), blob.size());
      }

      /**
       * @brief  Accessor for size.
       * @return
       */
      constexpr size_t size() const noexcept { return blob.size(); }

      /**
       * @brief Accessor for blob.
       * @return vector of bytes.
       */
      std::vector<uint8_t> data() noexcept { return blob; }

      /**
       * @brief Accessor for blob.
       * @return const reference to vector of bytes.
       */
      std::vector<uint8_t> const& data() const noexcept { return blob; }
    };

    /**
     * Secure equal comparator for private keys.
     * @param lhs
     * @param rhs
     * @return
     */
    inline bool operator==(PrivateKey const& lhs, PrivateKey const& rhs) {
      return secure::compare(lhs.data(), rhs.data());
    }

    /**
     * Secure not equal comparator for private keys.
     * @param lhs
     * @param rhs
     * @return
     */
    inline bool operator!=(PrivateKey const& lhs, PrivateKey const& rhs) {
      return !operator==(lhs, rhs);
    }
  }  // namespace crypto
}  // namespace iroha

#endif  //  IROHA_PRIVATE_KEY_HPP_
