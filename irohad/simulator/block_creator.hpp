/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_CREATOR_HPP
#define IROHA_BLOCK_CREATOR_HPP

#include <rxcpp/rx.hpp>
#include "simulator/block_creator_common.hpp"

namespace iroha {
  namespace simulator {

    /**
     * Interface for creating blocks from proposal
     */
    class BlockCreator {
     public:
      /**
       * Creates a block from given proposal and round
       */
      virtual void processVerifiedProposal(
          const std::shared_ptr<shared_model::interface::Proposal> &proposal,
          const consensus::Round &round) = 0;

      /**
       * Emit blocks made from proposals
       */
      virtual rxcpp::observable<BlockCreatorEvent> onBlock() = 0;

      virtual ~BlockCreator() = default;
    };
  }  // namespace simulator
}  // namespace iroha

#endif  // IROHA_BLOCK_CREATOR_HPP
