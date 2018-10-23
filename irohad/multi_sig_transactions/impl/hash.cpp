/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/hash.hpp"

#include <functional>
#include <string>

#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"

namespace iroha {
  namespace model {

    size_t PointerBatchHasher::operator()(const DataType &batch) const {
      return std::hash<std::string>{}(batch->reducedHash().hex());
    }

    std::size_t PeerHasher::operator()(
        const std::shared_ptr<shared_model::interface::Peer> &obj) const {
      return std::hash<std::string>{}(obj->address() + obj->pubkey().hex());
    }

  }  // namespace model
}  // namespace iroha
