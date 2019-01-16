/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
