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

#ifndef IROHA_YAC_CRYPTO_PROVIDER_HPP
#define IROHA_YAC_CRYPTO_PROVIDER_HPP

#include "consensus/yac/yac_hash_provider.hpp" // for YacHash (passed by copy)

namespace iroha {
  namespace consensus {
    namespace yac {

      struct CommitMessage;
      struct RejectMessage;
      struct VoteMessage;

      class YacCryptoProvider {
       public:
        /**
         * Verify signatory of message
         * @param msg - for verification
         * @return true if signature correct
         */
        virtual bool verify(CommitMessage msg) = 0;

        /**
         * Verify signatory of message
         * @param msg - for verification
         * @return true if signature correct
         */
        virtual bool verify(RejectMessage msg) = 0;

        /**
         * Verify signatory of message
         * @param msg - for verification
         * @return true if signature correct
         */
        virtual bool verify(VoteMessage msg) = 0;

        /**
         * Generate vote for provided hash;
         * @param hash - hash for signing
         * @return vote
         */
        virtual VoteMessage getVote(YacHash hash) = 0;

        virtual ~YacCryptoProvider() = default;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_CRYPTO_PROVIDER_HPP
