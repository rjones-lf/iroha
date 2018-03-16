/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_YAC_PROPOSAL_STORAGE_HPP
#define IROHA_YAC_PROPOSAL_STORAGE_HPP

#include <memory>
#include <boost/optional.hpp>
#include <vector>

#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "consensus/yac/storage/storage_result.hpp"
#include "consensus/yac/storage/yac_block_storage.hpp"
#include "consensus/yac/storage/yac_common.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      struct VoteMessage;

      /**
       * Class for storing votes related to given proposal hash
       * and gain information about commits/rejects for those hash
       */
      class YacProposalStorage {
       private:
        // --------| private api |--------

        /**
         * Find block index with provided parameters,
         * if those store absent - create new
         * @param proposal_hash - hash of proposal
         * @param block_hash - hash of block
         * @return iterator to storage
         */
        auto findStore(ProposalHash proposal_hash, BlockHash block_hash);

       public:
        // --------| public api |--------

        YacProposalStorage(
            ProposalHash hash,
            uint64_t peers_in_round,
            std::shared_ptr<SupermajorityChecker> supermajority_checker =
                std::make_shared<SupermajorityCheckerImpl>());

        /**
         * Try to insert vote to storage
         * @param vote - object for insertion
         * @return result, that contains actual state of storage.
         * Nullopt if not inserted, possible reasons - duplication,
         * wrong proposal hash.
         */
        boost::optional<Answer> insert(VoteMessage vote);

        /**
         * Insert bundle of messages into storage
         * @param messages - collection of messages
         * @return result, that contains actual state of storage,
         * after insertion of all votes.
         */
        boost::optional<Answer> insert(std::vector<VoteMessage> messages);

        /**
         * Provides hash assigned for storage
         */
        ProposalHash getProposalHash();

        /**
         * @return current state of storage
         */
        boost::optional<Answer> getState() const;

       private:
        // --------| private api |--------

        /**
         * Possible to insert vote
         * @param msg - vote for insertion
         * @return true if possible
         */
        bool shouldInsert(const VoteMessage &msg);

        /**
         * Is this vote valid for insertion in proposal storage
         * @param vote_hash - hash for verification
         * @return true if it may be applied
         */
        bool checkProposalHash(ProposalHash vote_hash);

        /**
         * Is this peer first time appear in this proposal storage
         * @return true, if peer unique
         */
        bool checkPeerUniqueness(const VoteMessage &msg);

        /**
         * Method try to find proof of reject.
         * This computes as
         * number of not voted peers + most frequent vote count < supermajority
         * @return answer with proof
         */
        boost::optional<Answer> findRejectProof();

        // --------| fields |--------

        /**
         * Current state of storage
         */
        boost::optional<Answer> current_state_;

        /**
         * Vector of block storages based on this proposal
         */
        std::vector<YacBlockStorage> block_storages_;

        /**
         * Hash of proposal
         */
        ProposalHash hash_;

        /**
         * Provide number of peers participated in current round
         */
        uint64_t peers_in_round_;

        /**
         * Provide functions to check supermajority
         */
        std::shared_ptr<SupermajorityChecker> supermajority_checker_;

        /**
         * Storage logger
         */
        logger::Logger log_;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_YAC_PROPOSAL_STORAGE_HPP
