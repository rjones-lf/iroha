/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_TXS4ROUND_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_TXS4ROUND_HPP_

#include "consensus/round.hpp"
#include "framework/integration_framework/fake_peer/types.hpp"

namespace integration_framework {
  namespace fake_peer {

    struct BatchesForRound final {
      using BatchesType = std::vector<
          std::shared_ptr<shared_model::interface::TransactionBatch>>;

      BatchesForRound(iroha::consensus::Round r, BatchesType b)
          : round(r), batches(b){};

      iroha::consensus::Round round;
      BatchesType batches;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_TXS4ROUND_HPP_ */
