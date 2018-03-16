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

#include "consensus/yac/storage/yac_common.hpp"

#include <algorithm>

#include "consensus/yac/messages.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      bool sameProposals(const std::vector<VoteMessage> &votes) {
        if (votes.empty()) {
          return false;
        }

        auto first = votes.at(0);
        return std::all_of(
            votes.begin(), votes.end(), [&first](const auto &current) {
              return first.hash.proposal_hash == current.hash.proposal_hash;
            });
      }

      boost::optional<ProposalHash> getProposalHash(
          const std::vector<VoteMessage> &votes) {
        auto &&hash = getHash(votes);
        if (hash) {
          return (*hash).proposal_hash;
        }
        return boost::none;
      }

      boost::optional<YacHash> getHash(const std::vector<VoteMessage> &votes) {
        if (not sameProposals(votes)) {
          return boost::none;
        }

        return votes.at(0).hash;
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
