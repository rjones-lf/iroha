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

#include "multi_sig_transactions/storage/mst_storage.hpp"
#include "model/operators/hash.hpp"
#include <unordered_map>

namespace iroha {
  class MstStorageStateImpl : public MstStorage {

   private:
    // -----------------------------| private API |-----------------------------

    /**
     * Return state of passed peer, if state doesn't exist, create empty
     * @param target_peer - peer for searching
     * @return valid iterator for state of peer
     */
    auto getState(ConstPeer &target_peer);

   public:
    // ----------------------------| interface API |----------------------------
    MstStorageStateImpl(ConstPeer &own_peer,
                        const CompleterType &completer);

    auto applyImpl(ConstPeer &target_peer, const MstState &new_state)
        -> decltype(apply(target_peer, new_state)) override;

    auto updateOwnStateImpl(const TransactionType &tx)
        -> decltype(updateOwnState(tx)) override;

    auto getExpiredTransactionsImpl(const TimeType &current_time)
        -> decltype(getExpiredTransactions(current_time)) override;

    auto getDiffStateImpl(ConstPeer &target_peer, const TimeType &current_time)
        -> decltype(getDiffState(target_peer, current_time)) override;

    auto getOwnStateImpl() const -> decltype(getOwnState()) override;

    virtual ~MstStorageStateImpl() = default;

   private:
    // ---------------------------| private fields |----------------------------

    std::unordered_map<ConstPeer, MstState, iroha::model::PeerHasher>
        peer_states_;
    ConstPeer own_peer_;
    const CompleterType completer_;

    MstState &own_state_;
  };
} // namespace iroha

#endif //IROHA_MST_STORAGE_IMPL_HPP
