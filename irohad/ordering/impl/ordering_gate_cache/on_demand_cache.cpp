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

OrderingGateCache::BatchesSetType OnDemandCache::clearFrontAndGet() {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  auto front = queue_.front();
  queue_.front().clear();
  return front;
}

void OnDemandCache::remove(
    const OrderingGateCache::BatchesSetType &remove_batches) {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  auto &front = queue_.front();

  for (auto batch : remove_batches) {
    front.erase(batch);
  }
}
void OnDemandCache::up() {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  auto popped = queue_.front();
  queue_.push(OrderingGateCache::BatchesSetType{});
  queue_.front().insert(popped.begin(), popped.end());
}

const OrderingGateCache::BatchesSetType &OnDemandCache::head() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return queue_.front();
}

const OrderingGateCache::BatchesSetType &OnDemandCache::tail() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return queue_.back();
}
