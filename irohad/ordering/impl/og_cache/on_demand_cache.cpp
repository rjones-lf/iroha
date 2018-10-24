/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/og_cache/on_demand_cache.hpp"

#include <iostream>

using namespace iroha::ordering::cache;

void OnDemandCache::addToBack(const OgCache::BatchesSetType &batches) {
  queue_.back().insert(batches.begin(), batches.end());
}

OgCache::BatchesSetType OnDemandCache::clearFrontAndGet() {
  auto front = queue_.front();
  queue_.front().clear();
  return front;
}

void OnDemandCache::remove(const OgCache::BatchesSetType &remove_batches) {
  auto &front = queue_.front();

  for (auto batch : remove_batches) {
    front.erase(batch);
  }
}
void OnDemandCache::up() {
  auto popped = queue_.front();
  queue_.push(OgCache::BatchesSetType{});
  queue_.front().insert(popped.begin(), popped.end());
}

const OgCache::BatchesSetType &OnDemandCache::head() const {
  return queue_.front();
}

const OgCache::BatchesSetType &OnDemandCache::tail() const {
  return queue_.back();
}
