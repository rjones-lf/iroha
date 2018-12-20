/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SUBSCRIPTION_WATCHER_HPP
#define IROHA_SUBSCRIPTION_WATCHER_HPP

#include <atomic>
#include <mutex>
#include <unordered_map>

#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-subscription.hpp>

namespace iroha {

  template <typename KeyType,
            typename ObservableTriggerType,
            typename Hash = std::hash<KeyType>>
  class SubscriptionWatcher {
   public:
    SubscriptionWatcher(
        rxcpp::observable<ObservableTriggerType> observable_trigger,
        int8_t max_counter_value)
        : kMaxCounterValue{max_counter_value} {
      observable_trigger.subscribe(subscription_trigger_, [this](const auto &) {
        this->incrementCounters();
      });
    }

    SubscriptionWatcher(const SubscriptionWatcher &sw) = delete;

    ~SubscriptionWatcher() {
      subscription_trigger_.unsubscribe();
    }

    void addSubscription(const KeyType &key,
                         rxcpp::subscription &subscription) {
      subscriptions_.emplace(key, SubscriptionAndCounter{subscription, 0});
    }

    void removeSubscription(const KeyType &key) {
      subscriptions_.erase(key);
    }

    void resetCounter(const KeyType &key) {
      std::lock_guard<std::mutex> lock{this->mutex_};
      auto it = subscriptions_.find(key);
      if (it != subscriptions_.cend()) {
        it->second.counter = 0;
      }
    }

   private:
    struct SubscriptionAndCounter {
      rxcpp::subscription &subscription;
      int8_t counter;
    };

    std::unordered_map<KeyType, SubscriptionAndCounter, Hash> subscriptions_;
    rxcpp::composite_subscription subscription_trigger_;
    std::mutex mutex_;
    const int8_t kMaxCounterValue;

    void incrementCounters() {
      std::lock_guard<std::mutex> lock{this->mutex_};
      auto it = subscriptions_.begin();
      while (it != subscriptions_.end()) {
        if (++it->second.counter >= kMaxCounterValue) {
          it = subscriptions_.erase(it);
        } else {
          ++it;
        }
      }
    }
  };

}  // namespace iroha

#endif  // IROHA_SUBSCRIPTION_WATCHER_HPP
