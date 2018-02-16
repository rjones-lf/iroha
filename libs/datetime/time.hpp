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

#ifndef IROHA_TIME_H
#define IROHA_TIME_H

#include <chrono>

namespace iroha {

  namespace time {

    /**
     * Returns current UNIX timestamp.
     * Represents number of milliseconds since epoch.
     */
    inline auto now() {
      return std::chrono::system_clock::now().time_since_epoch()
          / std::chrono::milliseconds(1);
    }

    /**
     * Return UNIX timestamp with given offset.
     * Represents number of milliseconds since epoch.
     */
    template <typename T>
    inline auto now(const T &offset) {
      return (std::chrono::system_clock::now().time_since_epoch() + offset)
          / std::chrono::milliseconds(1);
    }

    using time_t = decltype(iroha::time::now());
  }  // namespace time
}  // namespace iroha

#endif  // IROHA_TIME_H
