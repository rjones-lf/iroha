/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONSENSUS_GATE_HPP
#define IROHA_CONSENSUS_GATE_HPP

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <rxcpp/rx.hpp>

#include "consensus/round.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Block;
    class Proposal;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace network {
    /// Current pair is valid
    struct PairValid {
      std::shared_ptr<shared_model::interface::Block> block;
    };

    /// Network votes for another pair and round
    struct VoteOther {
      std::shared_ptr<shared_model::interface::Block> block;
    };

    /// Reject on proposal
    struct ProposalReject {
      consensus::Round round;
    };

    /// Reject on block
    struct BlockReject {
      consensus::Round round;
    };

    /// Agreement on <None, None>
    struct AgreementOnNone {
      consensus::Round round;
    };

    /**
     * Public api of consensus module
     */
    class ConsensusGate {
     public:
      using Round = consensus::Round;
      /**
       * Providing data for consensus for voting
       * @param block is the block for which current node is voting
       */
      virtual void vote(
          boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
              proposal,
          boost::optional<std::shared_ptr<shared_model::interface::Block>>
              block,
          Round round) = 0;

      // TODO(@l4l) 10/10/18: IR-1749 move instantiation to separate cpp
      using GateObject = boost::variant<PairValid,
                                        VoteOther,
                                        ProposalReject,
                                        BlockReject,
                                        AgreementOnNone>;

      /**
       * @return emit gate responses
       */
      virtual rxcpp::observable<GateObject> onOutcome() = 0;

      virtual ~ConsensusGate() = default;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_CONSENSUS_GATE_HPP
