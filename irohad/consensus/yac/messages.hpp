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

#ifndef IROHA_MESSAGES_HPP
#define IROHA_MESSAGES_HPP

#include <vector>

#include "consensus/yac/yac_hash_provider.hpp"  // for YacHash
#include "interfaces/common_objects/signature.hpp"
#include "utils/string_builder.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * VoteMessage represents voting for some block;
       */
      struct VoteMessage {
        YacHash hash;
        std::shared_ptr<shared_model::interface::Signature> signature;

        bool operator==(const VoteMessage &rhs) const {
          return hash == rhs.hash and *signature == *rhs.signature;
        }

        bool operator!=(const VoteMessage &rhs) const {
          return not(*this == rhs);
        }

        std::string toString() const {
          return shared_model::detail::PrettyStringBuilder()
              .init("VoteMessage")
              .append("yac hash", hash.toString())
              .append("signature",
                      signature ? signature->toString() : "not set")
              .finalize();
        }
      };

      /**
       * CommitMsg means consensus on cluster achieved.
       * All nodes deals on some solution
       */
      struct CommitMessage {
        explicit CommitMessage(std::vector<VoteMessage> votes)
            : votes(std::move(votes)) {}

        std::vector<VoteMessage> votes;

        bool operator==(const CommitMessage &rhs) const {
          return votes == rhs.votes;
        }

        std::string toString() const {
          return shared_model::detail::PrettyStringBuilder()
              .init("CommitMessage")
              .appendAll(
                  "votes", votes, [](auto vote) { return vote.toString(); })
              .finalize();
        }
      };

      /**
       * Reject means that there is impossible
       * to collect supermajority for any block
       */
      struct RejectMessage {
        explicit RejectMessage(std::vector<VoteMessage> votes)
            : votes(std::move(votes)) {}

        std::vector<VoteMessage> votes;

        bool operator==(const RejectMessage &rhs) const {
          return votes == rhs.votes;
        }

        std::string toString() const {
          return shared_model::detail::PrettyStringBuilder()
              .init("RejectMessage")
              .appendAll(
                  "votes", votes, [](auto vote) { return vote.toString(); })
              .finalize();
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_MESSAGES_HPP
