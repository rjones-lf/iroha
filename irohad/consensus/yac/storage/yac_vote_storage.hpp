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

#ifndef IROHA_YAC_VOTE_STORAGE_HPP
#define IROHA_YAC_VOTE_STORAGE_HPP

#include <memory>
#include <boost/optional.hpp>
#include <unordered_set>
#include <vector>

#include "consensus/yac/messages.hpp"  // because messages passed by value
#include "consensus/yac/storage/storage_result.hpp"  // for Answer
#include "consensus/yac/storage/yac_common.hpp"      // for ProposalHash

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacProposalStorage;

      /**
       * Class provide storage for votes and useful methods for it.
       */
      class YacVoteStorage {
       private:
        // --------| private api |--------

        /**
         * Retrieve iterator for storage with parameters hash
         * @param hash - object for finding
         * @return iterator to proposal storage
         */
        auto getProposalStorage(ProposalHash hash);

        /**
         * Find existed proposal storage or create new if required
         * @param msg - vote for finding
         * @param peers_in_round - number of peer required
         * for verify supermajority;
         * This parameter used on creation of proposal storage
         * @return - iter for required proposal storage
         */
        auto findProposalStorage(const VoteMessage &msg,
                                 uint64_t peers_in_round);

       public:
        // --------| public api |--------

        /**
         * Insert vote in storage
         * @param msg - current vote message
         * @param peers_in_round - number of peers participated in round
         * @return structure with result of inserting. Nullopt if mgs not valid.
         */
        boost::optional<Answer> store(VoteMessage msg,
                                       uint64_t peers_in_round);

        /**
         * Insert commit in storage
         * @param commit - message with votes
         * @param peers_in_round - number of peers in current consensus round
         * @return structure with result of inserting.
         * Nullopt if commit not valid.
         */
        boost::optional<Answer> store(CommitMessage commit,
                                       uint64_t peers_in_round);

        /**
         * Insert reject message in storage
         * @param reject - message with votes
         * @param peers_in_round - number of peers in current consensus round
         * @return structure with result of inserting.
         * Nullopt if reject not valid.
         */
        boost::optional<Answer> store(RejectMessage reject,
                                       uint64_t peers_in_round);

        /**
         * Provide status about closing round with parameters hash
         * @param hash - target hash of round
         * @return true, if rould closed
         */
        bool isHashCommitted(ProposalHash hash);

        /**
         * Method provide state of processing for concrete hash
         * @param hash - target tag
         * @return value attached to parameter's hash. Default is false.
         */
        bool getProcessingState(const ProposalHash &hash);

        /**
         * Mark hash as processed.
         * @param hash - target tag
         */
        void markAsProcessedState(const ProposalHash &hash);

       private:
        // --------| private api |--------

        /**
         * Insert votes in storage
         * @param votes - collection for insertion
         * @param peers_in_round - number of peers in current round
         * @return answer after insertion collection
         */
        boost::optional<Answer> insert_votes(std::vector<VoteMessage> &votes,
                                              uint64_t peers_in_round);

        // --------| fields |--------

        /**
         * Active proposal storages
         */
        std::vector<YacProposalStorage> proposal_storages_;

        /**
         * Processing set provide user flags about processing some hashes.
         * If hash exists <=> processed
         */
        std::unordered_set<ProposalHash> processing_state_;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_YAC_VOTE_STORAGE_HPP
