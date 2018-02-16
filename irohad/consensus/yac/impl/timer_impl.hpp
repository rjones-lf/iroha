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

#ifndef IROHA_TIMER_IMPL_HPP
#define IROHA_TIMER_IMPL_HPP

#include <rxcpp/rx.hpp>
#include "consensus/yac/timer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class TimerImpl : public Timer {
       public:
        TimerImpl() = default;
        TimerImpl(const TimerImpl &) = delete;
        TimerImpl &operator=(const TimerImpl &) = delete;

        void invokeAfterDelay(uint64_t millis,
                              std::function<void()> handler) override;
        void deny() override;

        ~TimerImpl() override;

       private:
        std::function<void()> handler_;

        rxcpp::observable<long> timer;
        rxcpp::composite_subscription handle;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_TIMER_IMPL_HPP
