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

#include "consensus/yac/impl/timer_impl.hpp"
#include <iostream>

namespace iroha {
  namespace consensus {
    namespace yac {
      TimerImpl::TimerImpl(
          std::function<rxcpp::observable<TimeoutType>()> invoke_delay)
          : invoke_delay_(std::move(invoke_delay)) {}

      void TimerImpl::invokeAfterDelay(std::function<void()> handler) {
        deny();
        handle_ = invoke_delay_().subscribe(
            [handler{std::move(handler)}](auto) { handler(); });
      }

      void TimerImpl::deny() {
        handle_.unsubscribe();
      }

      TimerImpl::~TimerImpl() {
        deny();
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
