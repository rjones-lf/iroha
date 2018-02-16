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

#include <string>
#include "model/block.hpp"      // for Block::HashType
#include "model/signature.hpp"  // for model::Signature

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacHash {
       public:
        YacHash(std::string proposal, std::string block)
            : proposal_hash(std::move(proposal)),
              block_hash(std::move(block)) {}

        YacHash() = default;

        /**
         * Hash computed from proposal
         */
        std::string proposal_hash;

        /**
         * Hash computed from block;
         */
        std::string block_hash;

        /**
         * Peer signature of block
         */
        model::Signature block_signature;

        bool operator==(const YacHash &obj) const {
          return proposal_hash == obj.proposal_hash
              and block_hash == obj.block_hash;
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
        virtual YacHash makeHash(const model::Block &block) const = 0;

        /**
         * Convert YacHash to model hash
         * @param hash - for converting
         * @return HashType of model hash
         */
        virtual model::Block::HashType toModelHash(
            const YacHash &hash) const = 0;

        virtual ~YacHashProvider() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_HASH_PROVIDER_HPP
