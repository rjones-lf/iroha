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

#ifndef IROHA_UTIL_HPP_
#define IROHA_UTIL_HPP_

#include <cstdio>  // for size_ts

namespace iroha {
  namespace secure {

    namespace util {
      /**
       * cast void ptr to volatile ptr of volatile T
       * @tparam T
       * @param mem
       * @return
       */
      template <typename T>
      constexpr T volatile *volatile void2volatileptr(void *mem) {
        return const_cast<volatile T *>(reinterpret_cast<T *>(mem));
      }
    }

    /**
     * @brief Secure erase of memory. Guarantees that given buffer will contain
     * zeroes after function completes.
     * @param[in] dest memory to be erased.
     * @param[in] size size of memory in bytes.
     */
    void erase(void *dest, size_t size) {
      char volatile *volatile p = util::void2volatileptr<char>(dest);

      if (size == 0) {
        return;
      }

      do {
        *p = 0;
      } while (*p++ && --size);
    }

    /**
     * @brief Secure compare of two chunks of memory. Resistant to timing
     * attacks.
     * @param lhs
     * @param rhs
     * @param size both chunks should be the same size.
     * @return true if memory chunks are equal, false otherwise.
     */
    bool compare(void *lhs, void *rhs, size_t size) {
      auto a = util::void2volatileptr<char>(lhs);
      auto b = util::void2volatileptr<char>(rhs);

      size_t s = 0;
      do {
        s |= (*a) ^ (*b);
      } while (*a++ && *b++ && --size);

      return s == 0;
    }

    /**
     * @brief Secure compare of two vectors. Resistant to timing
     * attacks.
     * @param lhs
     * @param rhs
     * @param size both vectors should be the same size.
     * @return true if vectors are equal, false otherwise.
     */
    bool compare(std::vector<uint8_t> const &lhs,
                 std::vector<uint8_t> const &rhs) {
      return lhs.size() == rhs.size()
          && compare((void *)lhs.data(), (void *)rhs.data(), lhs.size());
    }

  }  // namespace secure
}  // namespace iroha

#endif  //  IROHA_UTIL_HPP_
