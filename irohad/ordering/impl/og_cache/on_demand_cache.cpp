/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/og_cache/on_demand_cache.hpp"
#include <iostream>
#include "on_demand_cache.hpp"

using namespace iroha::ordering::cache;

void OnDemandCache::addToBack(const OgCache::BatchesListType &batches) {
  queue_.back().insert(batches.begin(), batches.end());
  for (const auto &batch : queue_.back()) {
    std::cout << batch->toString();
  }
  std::cout << std::endl;
}

OgCache::BatchesListType OnDemandCache::clearFrontAndGet() {
  auto front = queue_.front();
  queue_.front().clear();
  return front;
}

void OnDemandCache::remove(const OgCache::BatchesListType &remove_batches) {
  queue_.front().erase(remove_batches.begin(), remove_batches.end());
}
void OnDemandCache::up() {
  auto popped = queue_.front();
  queue_.push(OgCache::BatchesListType{});
  queue_.front().insert(popped.begin(), popped.end());
}

const OgCache::BatchesListType &OnDemandCache::front() const {
  return queue_.front();
}

const OgCache::BatchesListType &OnDemandCache::back() const {
  return queue_.back();
}
