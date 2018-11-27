/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_OS_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_OS_NETWORK_NOTIFIER_HPP_

#include "network/ordering_service_transport.hpp"

#include <rxcpp/rx.hpp>

namespace shared_model {
  namespace interface {
    class TransactionBatch;
  }
}  // namespace shared_model

namespace integration_framework {
  namespace fake_peer {

    class OsNetworkNotifier final
        : public iroha::network::OrderingServiceNotification {
     public:
      using TransactionBatch = shared_model::interface::TransactionBatch;
      using TransactionBatchPtr = std::shared_ptr<TransactionBatch>;

      void onBatch(std::unique_ptr<TransactionBatch> batch) override;

      rxcpp::observable<TransactionBatchPtr> get_observable();

     private:
      rxcpp::subjects::subject<TransactionBatchPtr> batches_subject_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* FAKE_PEER_OS_NETWORK_NOTIFIER_HPP_ */
