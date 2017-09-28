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

#ifndef IROHA_PRNG_HPP_
#define IROHA_PRNG_HPP_

#include <algorithm>
#include <functional>
#include <random>

namespace iroha {
  namespace secure {

    /**
     * @class prng
     * @brief Pseudo Random Number Generator, seeded by random uint64
     * @tparam IntType arbitrary integer type, such as uint8 or uint64.
     */
    template <typename IntType>
    class prng {
     public:
      /**
       * @brief Constructor. PRNG returns values in range [min, max)
       * @param min
       * @param max
       */
      explicit prng(IntType min = std::numeric_limits<IntType>::min(),
                    IntType max = std::numeric_limits<IntType>::max())
          : dis(min, max), tw(/* seed engine with */ true_random_uint64()) {}

      /**
       * @brief Get random number of 'IntType'.
       * @return random integer number with uniform distribution.
       */
      IntType get() { return dis(tw); }

      /**
       * @brief Get 'size' random numbers.
       * @param size the size of returned vector.
       * @return vector of random numbers.
       */
      std::vector<IntType> get(size_t size) {
        std::vector<IntType> v(size);
        std::generate_n(v.begin(), size, [this]() { return dis(tw); });
        return v;
      }

      /**
       * @brief Cryptographically secure true random 64 bit generator, which can
       * be used for seeding. 32-bit seeds are not safe to use.
       * @return 64 bits of true randomness.
       */
      static uint64_t true_random_uint64() {
        // rd() returns only 32 bits, we want 64.
        uint64_t seed = rd();        // top 32 random bits
        seed = (seed << 32) | rd();  // bottom 32 random bits
        return seed;
      }

     private:
      static std::random_device rd;
      std::uniform_int_distribution<IntType> dis;
      std::mt19937_64 tw;
    };
  }  // namespace secure
}  // namespace iroha

#endif  //  IROHA_PRNG_HPP_
