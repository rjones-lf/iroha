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

#ifndef IROHA_HASH_HPP
#define IROHA_HASH_HPP

#include <functional>
#include <string>
#include "interfaces/common_objects/peer.hpp"
#include "multi_sig_transactions/mst_types.hpp"

namespace iroha {
  namespace model {
    /**
     * Hash calculation factory for batch
     */
    template <typename BatchType>
    class PointerBatchHasher {
     public:
      size_t operator()(const BatchType &batch) const {
        return string_hasher(batch->reducedHash().hex());
      }

     private:
      std::hash<std::string> string_hasher;
    };

    /**
     * Hasing of peer object
     */
    class PeerHasher {
     public:
      std::size_t operator()(
          const std::shared_ptr<shared_model::interface::Peer> &obj) const {
        return hasher(obj->address() + obj->pubkey().hex());
      }

     private:
      std::hash<std::string> hasher;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_HASH_HPP
