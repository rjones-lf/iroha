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

#ifndef IROHA_SIGNATURES_RESPONSE_HPP
#define IROHA_SIGNATURES_RESPONSE_HPP

#include <vector>

#include "crypto/keypair.hpp"
#include "model/query_response.hpp"

namespace iroha {
  namespace model {

    /**
     * Provide response with signatories attached to the account
     */
    struct SignatoriesResponse : public QueryResponse {
      /**
       * Vector with all public keys attached to account
       */
      std::vector<pubkey_t> keys{};
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_SIGNATURES_RESPONSE_HPP
