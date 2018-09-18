/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SYNCHRONIZER_COMMON_HPP
#define IROHA_SYNCHRONIZER_COMMON_HPP

#include <utility>

#include <rxcpp/rx-observable.hpp>

#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace synchronizer {

    /**
     * Chain of block(s), which was either committed directly by this peer or
     * downloaded from another; contains zero or more blocks depending on
     * synchronization outcome
     */
    using Chain =
        rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>;

    /**
     * Outcome, which was decided by synchronizer based on consensus result and
     * current local ledger state
     */
    enum class SynchronizationOutcomeType { kCommit, kReject };

    /**
     * Event, which is emitted by synchronizer, when it receives and processes
     * commit
     */
    struct SynchronizationEvent {
      Chain synced_blocks;
      SynchronizationOutcomeType sync_outcome;
    };

  }  // namespace synchronizer
}  // namespace iroha

#endif  // IROHA_SYNCHRONIZER_COMMON_HPP
