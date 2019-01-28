/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/ordering_service_network_notifier.hpp"

#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace integration_framework {

  void OsNetworkNotifier::onBatch(std::unique_ptr<TransactionBatch> batch) {
    std::shared_ptr<TransactionBatch> batch_ptr = std::move(batch);
    batches_subject_.get_subscriber().on_next(std::move(batch_ptr));
  }

  rxcpp::observable<OsNetworkNotifier::TransactionBatchSPtr>
  OsNetworkNotifier::get_observable() {
    return batches_subject_.get_observable();
  }

}  // namespace integration_framework
