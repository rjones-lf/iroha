/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_HASH_HPP
#define IROHA_HASH_HPP

#include "multi_sig_transactions/mst_types.hpp"

namespace iroha {
  namespace model {
    /**
     * Hash calculation factory for batch
     */
    class PointerBatchHasher {
     public:
      size_t operator()(const DataType &batch) const;
    };

    /**
     * Hasing of peer object
     */
    class PeerHasher {
     public:
      std::size_t operator()(
          const std::shared_ptr<shared_model::interface::Peer> &obj) const;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_HASH_HPP
