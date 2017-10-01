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
#include <unordered_map>

namespace iroha {
  class MstStorageStateImpl : public MstStorage {
   public:
    MstStorageStateImpl(ConstPeer own_peer);

// ------------------------------| interface API |------------------------------

    void applyImpl(ConstPeer &target_peer, MstState &new_state) override;

    void updateOwnStateImpl(TransactionType tx) override;

    MstState getDiffStateImpl(ConstPeer &target_peer) const override;

    virtual ~MstStorageStateImpl() = default;

   private:
// -------------------------------| private API |-------------------------------

    class PeerHasher {
     public:
      std::size_t operator()(ConstPeer &obj) const {
        return hasher(obj.address + obj.pubkey.to_string());
      }
     private:
      std::hash<std::string> hasher;
    };

// -----------------------------| private fileds |------------------------------

    std::unordered_map<ConstPeer, MstState, PeerHasher> peer_states_;

    ConstPeer own_peer_;
  };
} // namespace iroha

#endif //IROHA_MST_STORAGE_IMPL_HPP
