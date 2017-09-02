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

#ifndef IROHA_YAC_COMMON_HPP
#define IROHA_YAC_COMMON_HPP

#include "consensus/yac/messages.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      using ProposalHash = decltype(YacHash::proposal_hash);

      using BlockHash = decltype(YacHash::block_hash);

      /**
       * Check that current number >= supermajority.
       * @param current - current number for validation
       * @param all - whole number (N)
       * @return true if belong supermajority
       */
      bool hasSupermajority(uint64_t current, uint64_t all);

      /**
       * Check if there is available reject proof.
       * Reject proof is proof that in current round
       * no one hash doesn't achieve supermajority.
       * @param frequent - number of times, that appears most frequent element
       * @param voted - all number of voted peers
       * @param all - number of peers in round
       * @return true, if reject
       */
      bool hasReject(uint64_t frequent, uint64_t voted, uint64_t all);

      /**
       * Check that all votes in collection has same proposal hash
       * @param votes - collection of votes
       * @return true, if proposals same
       */
      bool sameProposals(const std::vector<VoteMessage> &votes);

      /**
       * Provide hash common for whole collection
       * @param votes - collection with votes
       * @return hash, if collection has same proposal hash, otherwise nullopt
       */
      nonstd::optional<ProposalHash>
      getProposalHash(const std::vector<VoteMessage> &votes);

      /**
       * Get common hash from collection
       * @param votes - collection with votes
       * @return hash, if collection elements have same hash, otherwise nullopt
       */
      nonstd::optional<YacHash> getHash(const std::vector<VoteMessage> &votes);
    } // namespace yac
  } // namespace consensus
} // namespace iroha
#endif //IROHA_YAC_COMMON_HPP
