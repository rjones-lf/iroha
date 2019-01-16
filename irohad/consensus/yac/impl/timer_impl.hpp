/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
        /// Delay observable type
        using TimeoutType = long;

        /**
         * Constructor
         * @param invoke_delay cold observable which specifies invoke strategy
         */
        explicit TimerImpl(
            std::function<rxcpp::observable<TimeoutType>()> invoke_delay);
        TimerImpl(const TimerImpl &) = delete;
        TimerImpl &operator=(const TimerImpl &) = delete;

        void invokeAfterDelay(std::function<void()> handler) override;
        void deny() override;

        ~TimerImpl() override;

       private:
        std::function<rxcpp::observable<TimeoutType>()> invoke_delay_;
        rxcpp::composite_subscription handle_;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_TIMER_IMPL_HPP
