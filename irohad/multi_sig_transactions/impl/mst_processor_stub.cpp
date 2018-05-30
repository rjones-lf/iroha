/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/mst_processor_stub.hpp"

using namespace iroha;

auto MstProcessorStub::propagateTransactionImpl(const DataType transaction)
    -> decltype(propagateTransaction(transaction)) {
  log_->error("Multisig transactions are disabled. Skipping transaction: {}",
              transaction->toString());
}

auto MstProcessorStub::onStateUpdateImpl() const -> decltype(onStateUpdate()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<std::shared_ptr<MstState>>();
}

auto MstProcessorStub::onPreparedTransactionsImpl() const
    -> decltype(onPreparedTransactions()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<DataType>();
}

auto MstProcessorStub::onExpiredTransactionsImpl() const
    -> decltype(onExpiredTransactions()) {
  log_->warn(
      "Multisig transactions are disabled, so MstProcessor observable won't "
      "emit any events");
  return rxcpp::observable<>::empty<DataType>();
}
