/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_common.hpp"

namespace iroha {
  namespace ordering {

    const consensus::RejectRoundType kFirstRejectRound = 0;

    consensus::RejectRoundType currentRejectRoundConsumer(
        consensus::RejectRoundType round) {
      return round + 2;
    }

    const consensus::RejectRoundType kNextRejectRoundConsumer =
        kFirstRejectRound + 1;

    const consensus::RejectRoundType kNextCommitRoundConsumer =
        kFirstRejectRound;

  }  // namespace ordering
}  // namespace iroha
