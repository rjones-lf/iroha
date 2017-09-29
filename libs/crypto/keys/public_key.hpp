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

#ifndef IROHA_PUBLIC_KEY_HPP
#define IROHA_PUBLIC_KEY_HPP

#include <array>
#include <common/types.hpp>
#include <cstring>
#include <secure/prng.hpp>
#include <secure/util.hpp>

namespace iroha {
  namespace crypto {
    /**
     * @class PublicKey
     * Represents public data, which could be distributed among anyone
     * and used for signature verification
     */
    class PublicKey final {
     private:
      std::string blob;

     public:
      PublicKey() = default;                      ///< default constructor
      PublicKey(PublicKey const&) = default;      ///< copy constructor
      PublicKey(PublicKey&&) noexcept = default;  ///< move constructor
      PublicKey& operator=(PublicKey const&) =
          default;  ///< copy assignment operator
      PublicKey& operator=(PublicKey&&) noexcept =
          default;  ///< move assignment operator

      /**
       * @brief  Accessor for size.
       * @return
       */
      const size_t size() const noexcept { return blob.size(); }

      /**
       * @brief Accessor for blob.
       * @return vector of bytes.
       */
      std::string data() noexcept { return blob; }

      /**
       * @brief Accessor for blob.
       * @return const reference to vector of bytes.
       */
      std::string const& data() const noexcept { return blob; }
    };

    /**
     * Secure equal comparator for public keys (why not?)
     * @param lhs
     * @param rhs
     * @return
     */
    inline bool operator==(PublicKey const& lhs, PublicKey const& rhs) {
      return secure::compare(lhs.data(), rhs.data());
    }

    /**
     * Secure not equal comparator for public keys.
     * @param lhs
     * @param rhs
     * @return
     */
    inline bool operator!=(PublicKey const& lhs, PublicKey const& rhs) {
      return !operator==(lhs, rhs);
    }
  }
}

#endif  // IROHA_PUBLIC_KEY_HPP
