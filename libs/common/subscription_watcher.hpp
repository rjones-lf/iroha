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

  /**
   * This class stores a map of any keys to pairs "subscription-counter". When
   * event, observer to which is passed to the ctor, happens, counters are
   * incremented. After counter reaches some maximum value, a related
   * subscription is terminated
   * @tparam KeyType - type of key in the map
   * @tparam ObservableTriggerType - type inside the observable, which
   * increments counters
   * @tparam Hash - how to take hash from the key
   */
  template <typename KeyType,
            typename ObservableTriggerType,
            typename Hash = std::hash<KeyType>>
  class SubscriptionWatcher {
   public:
    /**
     * @param observable_trigger - observable, which events trigger counters
     * incrementation
     * @param max_counter_value - value, after which subscription is terminated
     */
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

    /**
     * Add a subscription to the collection
     * @param key of this subscription
     * @param subscription itself
     */
    void addSubscription(const KeyType &key,
                         rxcpp::subscription &subscription) {
      subscriptions_.emplace(key, SubscriptionAndCounter{subscription, 0});
    }

    /**
     * Remove a subscription from the collection
     * @param key of the subscription
     */
    void removeSubscription(const KeyType &key) {
      subscriptions_.erase(key);
    }

    /**
     * Reset one of the counters to 0
     * @param key of subscription to be resetted
     */
    void resetCounter(const KeyType &key) {
      // should be thread-safe, so that counters incrementation and reset would
      // not race
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

    /**
     * Increment all the counters and remove subscriptions, which have exceeded
     * maximum values
     */
    void incrementCounters() {
      // should be thread-safe, so that counters incrementation and reset would
      // not race
      std::lock_guard<std::mutex> lock{this->mutex_};
      auto it = subscriptions_.begin();
      while (it != subscriptions_.end()) {
        if (++it->second.counter >= kMaxCounterValue) {
          it->second.subscription.unsubscribe();
          it = subscriptions_.erase(it);
        } else {
          ++it;
        }
      }
    }
  };

}  // namespace iroha

#endif  // IROHA_SUBSCRIPTION_WATCHER_HPP
