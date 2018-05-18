/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_VERIFIED_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROTO_VERIFIED_PROPOSAL_HPP

#include "backend/protobuf/proto_proposal_template.hpp"
#include "interfaces/iroha_internal/verified_proposal.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Verified proposal is the proposal which contains only stateful valid
     * transactions. Transactions set might be empty.
     */
    using VerifiedProposal = ProtoProposalTemplate<interface::VerifiedProposal>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_VERIFIED_PROPOSAL_HPP
