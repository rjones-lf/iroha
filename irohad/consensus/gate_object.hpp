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

    struct Synchronizable {
      shared_model::interface::types::PublicKeyCollectionType public_keys;
      consensus::Round round;
      Synchronizable(
          shared_model::interface::types::PublicKeyCollectionType _public_keys,
          consensus::Round _round)
          : public_keys(std::move(_public_keys)), round(std::move(_round)) {}
    };

    /// Network votes for another pair and round
    struct VoteOther : public Synchronizable {
      shared_model::interface::types::HashType hash;
      VoteOther(
          shared_model::interface::types::PublicKeyCollectionType _public_keys,
          consensus::Round _round,
          shared_model::interface::types::HashType _hash)
          : Synchronizable(std::move(_public_keys), std::move(_round)),
            hash(std::move(_hash)) {}
    };

    /// Reject on proposal
    struct ProposalReject : public Synchronizable {
      using Synchronizable::Synchronizable;
    };

    /// Reject on block
    struct BlockReject : public Synchronizable {
      using Synchronizable::Synchronizable;
    };

    /// Agreement on <None, None>
    struct AgreementOnNone : public Synchronizable {
      using Synchronizable::Synchronizable;
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
