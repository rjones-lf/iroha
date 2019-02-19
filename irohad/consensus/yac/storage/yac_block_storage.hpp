/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_YAC_BLOCK_VOTE_STORAGE_HPP
#define IROHA_YAC_BLOCK_VOTE_STORAGE_HPP

#include <memory>
#include <vector>

#include <boost/optional.hpp>
#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "consensus/yac/outcome_messages.hpp"
#include "consensus/yac/storage/storage_result.hpp"
#include "consensus/yac/yac_types.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      /**
       * Class provide storage of votes for one block.
       */
      class YacBlockStorage {
       private:
        // --------| fields |--------

        /**
         * All votes stored in block store
         */
        std::vector<VoteMessage> votes_;

       public:
        YacBlockStorage(
            YacHash hash,
            PeersNumberType peers_in_round,
            std::shared_ptr<SupermajorityChecker> supermajority_checker =
                std::make_shared<SupermajorityCheckerImpl>());

        /**
         * Try to insert vote to storage
         * @param msg - vote for insertion
         * @return actual state of storage,
         * boost::none when storage doesn't have supermajority
         */
        boost::optional<Answer> insert(VoteMessage msg);

        /**
         * Insert vector of votes to current storage
         * @param votes - bunch of votes for insertion
         * @return state of storage after insertion last vote,
         * boost::none when storage doesn't have supermajority
         */
        boost::optional<Answer> insert(std::vector<VoteMessage> votes);

        /**
         * @return votes attached to storage
         */
        std::vector<VoteMessage> getVotes() const;

        /**
         * @return number of votes attached to storage
         */
        size_t getNumberOfVotes() const;

        /**
         * @return current block store state
         */
        boost::optional<Answer> getState();

        /**
         * Verify that passed vote contains in storage
         * @param msg  - vote for finding
         * @return true, if contains
         */
        bool isContains(const VoteMessage &msg) const;

        /**
         * Provide key attached to this storage
         */
        YacHash getStorageKey() const;

       private:
        // --------| private api |--------

        /**
         * Verify uniqueness of vote in storage
         * @param msg - vote for verification
         * @return true if vote doesn't appear in storage
         */
        bool uniqueVote(VoteMessage &vote);

        /**
         * Verify that vote has the same hash attached as the storage
         * @param vote - vote to be checked
         * @return true, if validation passed
         */
        bool validScheme(VoteMessage &vote);

        // --------| fields |--------

        /**
         * Key of the storage; currently it's yac hash
         */
        YacHash storage_key_;

        /**
         * Number of peers in current round
         */
        PeersNumberType peers_in_round_;

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
#endif  // IROHA_YAC_BLOCK_VOTE_STORAGE_HPP
