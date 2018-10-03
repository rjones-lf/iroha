/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/og_cache/on_demand_cache.hpp"
#include "on_demand_cache.hpp"

using namespace iroha::ordering::cache;

void OnDemandCache::addToBack(
    std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
  queue_.back().insert(batch);
}

OgCache::BatchesListType OnDemandCache::dequeue() {
  auto &front = queue_.front();
  queue_.pop();
  return front;
}

void OnDemandCache::remove(const OgCache::BatchesListType &remove_batches) {
  queue_.front().erase(remove_batches.begin(), remove_batches.end());
}
