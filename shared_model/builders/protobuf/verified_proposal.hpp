/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VERIFIED_PROPOSAL_HPP
#define IROHA_VERIFIED_PROPOSAL_HPP

#include "backend/protobuf/verified_proposal.hpp"
#include "builders/protobuf/builder_templates/proposal_template.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    using VerifiedProposalBuilder =
        TemplateProposalBuilder<0,
                                validation::DefaultVerifiedProposalValidator,
                                VerifiedProposal>;
  }
}  // namespace shared_model

#endif  // IROHA_VERIFIED_PROPOSAL_HPP
