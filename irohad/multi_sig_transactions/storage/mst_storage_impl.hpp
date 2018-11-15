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

#ifndef IROHA_MST_STORAGE_IMPL_HPP
#define IROHA_MST_STORAGE_IMPL_HPP

#include <unordered_map>
#include "multi_sig_transactions/hash.hpp"
#include "multi_sig_transactions/storage/mst_storage.hpp"

namespace iroha {
  class MstStorageStateImpl : public MstStorage {
   private:
    // -----------------------------| private API |-----------------------------

    /**
     * Return state of a peer by its public key. If state doesn't exist, create
     * new empty state and return it.
     * @param target_peer_key - public key of the peer for searching
     * @return valid iterator for state of peer
     */
    auto getState(const shared_model::crypto::PublicKey &target_peer_key);

   public:
    // ----------------------------| interface API |----------------------------
    explicit MstStorageStateImpl(const CompleterType &completer);

    auto applyImpl(const shared_model::crypto::PublicKey &target_peer_key,
                   const MstState &new_state)
        -> decltype(apply(target_peer_key, new_state)) override;

    auto updateOwnStateImpl(const DataType &tx)
        -> decltype(updateOwnState(tx)) override;

    auto getExpiredTransactionsImpl(const TimeType &current_time)
        -> decltype(getExpiredTransactions(current_time)) override;

    auto getDiffStateImpl(
        const shared_model::crypto::PublicKey &target_peer_key,
        const TimeType &current_time)
        -> decltype(getDiffState(target_peer_key, current_time)) override;

    auto whatsNewImpl(ConstRefState new_state) const
        -> decltype(whatsNew(new_state)) override;

   private:
    // ---------------------------| private fields |----------------------------

    const CompleterType completer_;
    std::unordered_map<shared_model::crypto::PublicKey,
                       MstState,
                       iroha::model::BlobHasher>
        peer_states_;
    MstState own_state_;
  };
}  // namespace iroha

#endif  // IROHA_MST_STORAGE_IMPL_HPP
