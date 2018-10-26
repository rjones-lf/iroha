/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/og_cache/on_demand_cache.hpp"

#include <iostream>

using namespace iroha::ordering::cache;

void OnDemandCache::addToBack(
    const OrderingGateCache::BatchesSetType &batches) {
  queue_.back().insert(batches.begin(), batches.end());
}

OrderingGateCache::BatchesSetType OnDemandCache::clearFrontAndGet() {
  auto front = queue_.front();
  queue_.front().clear();
  return front;
}

void OnDemandCache::remove(
    const OrderingGateCache::BatchesSetType &remove_batches) {
  auto &front = queue_.front();

  for (auto batch : remove_batches) {
    front.erase(batch);
  }
}
void OnDemandCache::up() {
  auto popped = queue_.front();
  queue_.push(OrderingGateCache::BatchesSetType{});
  queue_.front().insert(popped.begin(), popped.end());
}

const OrderingGateCache::BatchesSetType &OnDemandCache::head() const {
  return queue_.front();
}

const OrderingGateCache::BatchesSetType &OnDemandCache::tail() const {
  return queue_.back();
}
