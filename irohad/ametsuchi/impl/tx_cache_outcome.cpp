/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/tx_cache_outcome.hpp"

using namespace iroha::ametsuchi;

const TxCacheOutcome::HashType &TxCacheOutcome::HashContainer::hash() const {
  return hash_;
}

const TxCacheOutcome::OutcomeType &TxCacheOutcome::outcome() const {
  return outcome_;
}
