/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SYNCHRONIZER_COMMON_HPP
#define IROHA_SYNCHRONIZER_COMMON_HPP

#include <utility>

#include <boost/variant.hpp>
#include <rxcpp/rx-observable.hpp>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/empty_block.hpp"

namespace iroha {
  namespace synchronizer {

    /**
     * Chain of block(s), which was either committed directly by this peer or
     * downloaded from another
     */
    using Chain =
        rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>;

    /**
     * Type of consensus outcome, received by synchronizer
     */
    enum class ConsensusOutcomeType {
      nonempty,
      empty,
      reject
    };

    /**
     * Event, which is emitted by synchronizer, when it receives commit
     */
    using SynchronizerCommitReceiveEvent = std::pair<Chain, ConsensusOutcomeType>;

  }  // namespace synchronizer
}  // namespace iroha

#endif  // IROHA_SYNCHRONIZER_COMMON_HPP
