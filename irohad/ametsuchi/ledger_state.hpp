/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_LEDGER_STATE_HPP
#define IROHA_LEDGER_STATE_HPP

#include <memory>

#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  using PeerList = std::vector<std::shared_ptr<shared_model::interface::Peer>>;

  struct LedgerState {
    std::shared_ptr<PeerList> ledger_peers;

    LedgerState(std::shared_ptr<PeerList> peers)
        : ledger_peers(std::move(peers)) {}
    LedgerState() = default;
  };
}  // namespace iroha

#endif  // IROHA_LEDGER_STATE_HPP
