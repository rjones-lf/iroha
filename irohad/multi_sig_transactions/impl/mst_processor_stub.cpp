/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/mst_processor_stub.hpp"

using namespace iroha;

auto MstProcessorStub::propagateBatchImpl(const DataType &batch)
    -> decltype(propagateBatch(batch)) {
  log_->error("Multisig transactions are disabled. Skipping batch: {}",
              batch->reducedHash().toString());
}

auto MstProcessorStub::onStateUpdateImpl() const -> decltype(onStateUpdate()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<std::shared_ptr<MstState>>();
}

auto MstProcessorStub::onPreparedBatchesImpl() const
    -> decltype(onPreparedBatches()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<DataType>();
}

auto MstProcessorStub::onExpiredBatchesImpl() const
    -> decltype(onExpiredBatches()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<DataType>();
}
