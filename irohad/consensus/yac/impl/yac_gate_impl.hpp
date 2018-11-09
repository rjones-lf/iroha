/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_GATE_IMPL_HPP
#define IROHA_YAC_GATE_IMPL_HPP

#include "consensus/yac/yac_gate.hpp"

#include <memory>

#include "consensus/consensus_block_cache.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace simulator {
    class BlockCreator;
  }

  namespace network {
    class BlockLoader;
  }

  namespace consensus {
    namespace yac {

      struct CommitMessage;
      class YacPeerOrderer;

      class YacGateImpl : public YacGate {
       public:
        YacGateImpl(std::shared_ptr<HashGate> hash_gate,
                    std::shared_ptr<YacPeerOrderer> orderer,
                    std::shared_ptr<YacHashProvider> hash_provider,
                    std::shared_ptr<simulator::BlockCreator> block_creator,
                    std::shared_ptr<consensus::ConsensusResultCache>
                        consensus_result_cache);
        void vote(
            boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
                proposal,
            boost::optional<std::shared_ptr<shared_model::interface::Block>>
                block,
            Round round) override;

        rxcpp::observable<GateObject> onOutcome() override;

       private:
        /**
         * Update current block with signatures from commit message
         * @param commit - commit message to get signatures from
         */
        void copySignatures(const CommitMessage &commit);

        rxcpp::observable<GateObject> handleCommit(const CommitMessage &msg);
        rxcpp::observable<GateObject> handleReject(const RejectMessage &msg);

        std::shared_ptr<HashGate> hash_gate_;
        std::shared_ptr<YacPeerOrderer> orderer_;
        std::shared_ptr<YacHashProvider> hash_provider_;
        std::shared_ptr<simulator::BlockCreator> block_creator_;

        std::shared_ptr<consensus::ConsensusResultCache>
            consensus_result_cache_;

        logger::Logger log_;

        boost::optional<std::shared_ptr<shared_model::interface::Block>>
            current_block_;
        YacHash current_hash_;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_GATE_IMPL_HPP
