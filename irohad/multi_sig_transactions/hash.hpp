/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_HASH_HPP
#define IROHA_HASH_HPP

#include <functional>
#include <string>

#include "cryptography/public_key.hpp"
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
