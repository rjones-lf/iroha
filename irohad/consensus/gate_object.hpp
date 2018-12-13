/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONSENSUS_GATE_OBJECT_HPP
#define CONSENSUS_GATE_OBJECT_HPP

#include <boost/variant.hpp>
#include "consensus/round.hpp"
#include "cryptography/hash.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace consensus {

    /// Current pair is valid
    struct PairValid {
      std::shared_ptr<shared_model::interface::Block> block;
      consensus::Round round;
    };

    /// Network votes for another pair and round
    struct VoteOther {
      shared_model::interface::types::PublicKeyCollectionType public_keys;
      shared_model::interface::types::HashType hash;
      consensus::Round round;
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

    using GateObject = boost::variant<PairValid,
                                      VoteOther,
                                      ProposalReject,
                                      BlockReject,
                                      AgreementOnNone>;

  }  // namespace consensus
}  // namespace iroha

extern template class boost::variant<iroha::consensus::PairValid,
                                     iroha::consensus::VoteOther,
                                     iroha::consensus::ProposalReject,
                                     iroha::consensus::BlockReject,
                                     iroha::consensus::AgreementOnNone>;

#endif  // CONSENSUS_GATE_OBJECT_HPP
