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

#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include "algorithm"

namespace iroha {
  namespace consensus {
    namespace yac {

      YacHash YacHashProviderImpl::makeHash(model::Block &block) {
        YacHash hash;
        // todo add proposal hash from block.proposal_hash
        hash.proposal_hash = std::string(block.hash.size(), 0);
        auto hex_hash = block.hash.to_hexstring();
        hash.proposal_hash = hex_hash;
        hash.block_hash = hex_hash;
        return hash;
      }

      model::Block::HashType YacHashProviderImpl::toModelHash(YacHash hash) {
        model::Block::HashType result;
        hexstringToArray(hash.block_hash, result);
        return result;
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
