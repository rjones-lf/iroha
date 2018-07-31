/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_PROPOSAL_GATE_IMPL_HPP
#define IROHA_YAC_PROPOSAL_GATE_IMPL_HPP

#include "network/proposal_gate.hpp"

#include "consensus/yac/yac_gate.hpp"
#include "consensus/yac/yac_peer_orderer.hpp"
#include "consensus/yac/yac_proposal_hash_provider.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacProposalGateImpl : public network::ProposalGate {
       public:
        YacProposalGateImpl(
            std::shared_ptr<HashGate> hash_gate,
            std::shared_ptr<YacPeerOrderer> orderer,
            std::shared_ptr<YacProposalHashProvider> hash_provider);

        expected::Result<void, std::string> vote(
            network::ProposalVote vote) override;

        rxcpp::observable<network::ProposalOutcomeType> outcomes() override;

       private:
        std::shared_ptr<HashGate> hash_gate_;
        std::shared_ptr<YacPeerOrderer> orderer_;
        std::shared_ptr<YacProposalHashProvider> hash_provider_;

        boost::optional<std::pair<YacHash, network::ProposalVote>>
            last_voted_proposal_;

        /**
         * Logger instance
         */
        logger::Logger log_;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PROPOSAL_GATE_IMPL_HPP
