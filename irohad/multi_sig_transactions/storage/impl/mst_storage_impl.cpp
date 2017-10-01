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
  MstStorageStateImpl::MstStorageStateImpl(ConstPeer own_peer)
      : MstStorage(), own_peer_(own_peer) {
    peer_states_.insert({own_peer_, MstState()});
  }

  void MstStorageStateImpl::applyImpl(ConstPeer &target_peer,
                                      MstState &new_state) {
    auto own_iter = peer_states_.find(own_peer_);
    auto found_iter = peer_states_.find(target_peer);
    if (found_iter == peer_states_.end()) {

      own_iter->second = own_iter->second + new_state;

      peer_states_.insert({target_peer, std::move(new_state)});
    } else {

      own_iter->second = own_iter->second + new_state;

      found_iter->second = found_iter->second + new_state;
    }
  }

  void MstStorageStateImpl::updateOwnStateImpl(TransactionType tx) {
    auto found_iter = peer_states_.find(own_peer_);
    found_iter->second += tx;
  }

  MstState MstStorageStateImpl::getDiffStateImpl(ConstPeer &target_peer) const {
    auto own_iter = peer_states_.find(own_peer_);
    auto target_iter = peer_states_.find(target_peer);
    if (target_iter == peer_states_.end()) {
      return own_iter->second;
    }
    return own_iter->second - target_iter->second;
  }
} // namespace iroha
