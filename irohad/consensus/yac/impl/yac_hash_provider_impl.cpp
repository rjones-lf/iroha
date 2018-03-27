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

#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      YacHash YacHashProviderImpl::makeHash(
          const shared_model::interface::Block &block) const {
        YacHash result;
        auto hex_hash = block.hash().hex();
        result.proposal_hash = hex_hash;
        result.block_hash = hex_hash;
        const auto &sig = *block.signatures().begin();
        result.block_signature = clone(*sig);
        return result;
      }

      shared_model::interface::types::HashType YacHashProviderImpl::toModelHash(
          const YacHash &hash) const {
        auto blob = shared_model::crypto::Blob::fromHexString(hash.block_hash);
        auto string_blob = shared_model::crypto::toBinaryString(blob);
        return shared_model::interface::types::HashType(string_blob);
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
