/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_PROPOSAL_HASH_PROVIDER_IMPL_HPP
#define IROHA_YAC_PROPOSAL_HASH_PROVIDER_IMPL_HPP

#include "consensus/yac/yac_proposal_hash_provider.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacProposalHashProviderImpl : public YacProposalHashProvider {
       public:
        YacHash makeHash(const network::ProposalVote &vote) const override;

        ProposalInfo makeProposalInfo(const YacHash &hash) const override;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PROPOSAL_HASH_PROVIDER_IMPL_HPP
