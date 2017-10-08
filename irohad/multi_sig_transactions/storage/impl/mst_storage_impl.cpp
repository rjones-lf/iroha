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

#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

namespace iroha {
// -------------------------------| private API |-------------------------------

  auto MstStorageStateImpl::getState(ConstPeer &target_peer) {
    auto target_state_iter = peer_states_.find(target_peer);
    if (target_state_iter == peer_states_.end()) {
      return peer_states_
          .insert({target_peer, MstState::empty(completer_)})
          .first;
    }
    return target_state_iter;
  }
// ------------------------------| interface API |------------------------------
  MstStorageStateImpl::MstStorageStateImpl(const ConstPeer own_peer,
                                           const CompleterType completer)
      : MstStorage(),
        own_peer_(own_peer),
        completer_(completer),
        own_state_(peer_states_.insert({own_peer_, MstState::empty(completer_)})
                       .first->second) {
  }

  auto MstStorageStateImpl::applyImpl(ConstPeer &target_peer,
                                      MstState &new_state)
  -> decltype(apply(target_peer, new_state)) {
    auto target_state_iter = getState(target_peer);
    target_state_iter->second += new_state;
    return own_state_ += new_state;
  }

  auto MstStorageStateImpl::updateOwnStateImpl(TransactionType tx)
  -> decltype(updateOwnState(tx)) {
    return own_state_ += tx;
  }

  auto MstStorageStateImpl::getExpiredTransactionsImpl(const TimeType &current_time)
  -> decltype(getExpiredTransactions(current_time)) {
    return own_state_.eraseByTime(current_time);
  }

  auto MstStorageStateImpl::getDiffStateImpl(ConstPeer &target_peer,
                                             const TimeType &current_time)
  -> decltype(getDiffState(target_peer, current_time)) {
    auto target_current_state_iter = getState(target_peer);
    auto new_diff_state = own_state_ - target_current_state_iter->second;
    new_diff_state.eraseByTime(current_time);
    return new_diff_state;
  }

} // namespace iroha
