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

#ifndef IROHA_YAC_TIMER_HPP
#define IROHA_YAC_TIMER_HPP

#include <functional>

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Interface provide timer for yac implementation
       */
      class Timer {
       public:
        /**
         * Invoke handler with class-specific strategy
         * @param handler - function, that will be invoked
         */
        virtual void invokeAfterDelay(std::function<void()> handler) = 0;

        /**
         * Stop timer
         */
        virtual void deny() = 0;

        virtual ~Timer() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_YAC_TIMER_HPP
