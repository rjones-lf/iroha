/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_gate_cache/on_demand_cache.hpp"

using namespace iroha::ordering::cache;

// TODO: IR-1864 13.11.18 kamilsa use nvi to separate business logic and locking
// logic

void OnDemandCache::addToBack(
    const OrderingGateCache::BatchesSetType &batches) {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  circ_buffer.back().insert(batches.begin(), batches.end());
}

void OnDemandCache::remove(
    const OrderingGateCache::BatchesSetType &remove_batches) {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  for (auto &batches : circ_buffer) {
    for (const auto &removed_batch : remove_batches) {
      batches.erase(removed_batch);
    };
  }
}

OrderingGateCache::BatchesSetType OnDemandCache::pop() {
  std::unique_lock<std::shared_timed_mutex> lock(mutex_);
  auto res = circ_buffer.front();
  // push empty set to remove front element
  circ_buffer.push_back(BatchesSetType{});
  return res;
}

const OrderingGateCache::BatchesSetType &OnDemandCache::head() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return circ_buffer.front();
}

const OrderingGateCache::BatchesSetType &OnDemandCache::tail() const {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return circ_buffer.back();
}
