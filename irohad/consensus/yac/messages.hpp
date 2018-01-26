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

#ifndef IROHA_MESSAGES_HPP
#define IROHA_MESSAGES_HPP

#include <vector>

#include "consensus/yac/yac_hash_provider.hpp"
#include "model/signature.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * VoteMessage represents voting for some block;
       */
      struct VoteMessage {
        YacHash hash;
        model::Signature signature;

          //TODO: I am not sure that rhs is a great variable name choice here. This is a comparison operator
          // for VoteMessages, so &vote or &voteMessage might be more meaningful to humans.
        bool operator==(const VoteMessage &rhs) const {
          return hash == rhs.hash and signature == rhs.signature;
        }

          //TODO: 同上
        bool operator!=(const VoteMessage &rhs) const {
          return not(*this == rhs);
        }
      };

      /**
       * CommitMsg means consensus by validators was achieved.
       * All nodes deals on some solution
       */
      struct CommitMessage {
        explicit CommitMessage(std::vector<VoteMessage> votes)
            : votes(std::move(votes)) {}

        std::vector<VoteMessage> votes;

        bool operator==(const CommitMessage &rhs) const {
          return votes == rhs.votes;
        }
      };

      /**
       * A RejectMessage means that it was impossible
       * to collect a supermajority of votes for any block
       * proposal in the current consensus round.
       */
      struct RejectMessage {
        explicit RejectMessage(std::vector<VoteMessage> votes)
            : votes(std::move(votes)) {}

        std::vector<VoteMessage> votes;

        bool operator==(const RejectMessage &rhs) const {
          return votes == rhs.votes;
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_MESSAGES_HPP
