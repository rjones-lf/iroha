/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_PROPOSAL_HASH_PROVIDER_HPP
#define IROHA_YAC_PROPOSAL_HASH_PROVIDER_HPP

#include "consensus/yac/yac_hash_provider.hpp"
#include "network/proposal_gate.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Converters for proposal data and yachash
       */
      class YacProposalHashProvider {
       public:
        /**
         * Make hash from proposal vote
         * @param vote - for hashing
         * @return hashed value of proposal vote
         */
        virtual YacHash makeHash(const network::ProposalVote &vote) const = 0;

        /**
         * Information required to load proposal
         */
        struct ProposalInfo {
          boost::optional<shared_model::interface::types::HashType> hash;
          ordering::transport::RoundType round;
        };

        /**
         * Make ProposalInfo from hash
         * @param hash - for converting
         * @return proposal information
         */
        virtual ProposalInfo makeProposalInfo(const YacHash &hash) const = 0;

        virtual ~YacProposalHashProvider() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PROPOSAL_HASH_PROVIDER_HPP
