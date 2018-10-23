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

#ifndef IROHA_YAC_HASH_PROVIDER_HPP
#define IROHA_YAC_HASH_PROVIDER_HPP

#include <memory>
#include <string>

#include "consensus/round.hpp"
#include "consensus/yac/storage/yac_common.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Signature;
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacHash {
       public:
        YacHash(Round round, ProposalHash proposal, BlockHash block)
            : vote_round{round},
              vote_hashes{std::move(proposal), std::move(block)} {}

        YacHash() = default;

        /**
         * Round, in which peer voted
         */
        Round vote_round;

        /**
         * Contains hashes of proposal and block, for which peer voted
         */
        struct VoteHashes {
          /**
           * Hash computed from proposal
           */
          ProposalHash proposal_hash;

          /**
           * Hash computed from block;
           */
          BlockHash block_hash;
        };
        VoteHashes vote_hashes;

        /**
         * Peer signature of block
         */
        std::shared_ptr<shared_model::interface::Signature> block_signature;

        bool operator==(const YacHash &obj) const {
          return vote_round == obj.vote_round
              and vote_hashes.proposal_hash == obj.vote_hashes.proposal_hash
              and vote_hashes.block_hash == obj.vote_hashes.block_hash;
        };

        bool operator!=(const YacHash &obj) const {
          return not(*this == obj);
        };
      };

      /**
       * Provide methods related to hash operations in ya consensus
       */
      class YacHashProvider {
       public:
        /**
         * Make hash from block
         * @param block - for hashing
         * @return hashed value of block
         */
        virtual YacHash makeHash(
            const shared_model::interface::Block &block) const = 0;

        /**
         * Convert YacHash to model hash
         * @param hash - for converting
         * @return HashType of model hash
         */
        virtual shared_model::interface::types::HashType toModelHash(
            const YacHash &hash) const = 0;

        virtual ~YacHashProvider() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_HASH_PROVIDER_HPP
