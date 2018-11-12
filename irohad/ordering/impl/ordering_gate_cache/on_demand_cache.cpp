/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_gate_cache/on_demand_cache.hpp"

using namespace iroha::ordering::cache;

void OnDemandCache::addToBack(
    const OrderingGateCache::BatchesSetType &batches) {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  queue_.back().insert(batches.begin(), batches.end());
}

void OnDemandCache::remove(
    const OrderingGateCache::BatchesSetType &remove_batches) {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  for (auto &batches : queue_) {
    for (const auto &removed_batch : remove_batches) {
      batches.erase(removed_batch);
    };
  }
}

OrderingGateCache::BatchesSetType OnDemandCache::pop() {
  auto res = queue_.front();
  // push empty set to remove front element
  queue_.push_back(BatchesSetType{});
  return res;
}

const OrderingGateCache::BatchesSetType &OnDemandCache::head() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return queue_.front();
}

const OrderingGateCache::BatchesSetType &OnDemandCache::tail() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return queue_.back();
}
