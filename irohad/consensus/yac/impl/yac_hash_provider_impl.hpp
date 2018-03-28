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

#ifndef IROHA_YAC_HASH_PROVIDER_IMPL_HPP
#define IROHA_YAC_HASH_PROVIDER_IMPL_HPP

#include "consensus/yac/yac_hash_provider.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacHashProviderImpl : public YacHashProvider {
       public:
        YacHash makeHash(
            const shared_model::interface::Block &block) const override;

        shared_model::interface::types::HashType toModelHash(
            const YacHash &hash) const override;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_YAC_HASH_PROVIDER_IMPL_HPP
