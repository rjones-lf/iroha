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

#ifndef IROHA_MST_STORAGE_HPP
#define IROHA_MST_STORAGE_HPP

#include "model/peer.hpp"
#include "multi_sig_transactions/storage/mst_state.hpp"

namespace iroha {
  class MstStorage {

    /**
     * Apply new state for peer
     * @param target_peer - key for for updating state
     * @param new_state - state with new data
     */
    virtual void apply(const model::Peer& target_peer, MstState new_state) = 0;

    /**
     * Return difference between own and target state
     */
    virtual MstState getDiffState(const model::Peer& target_peer) = 0;

    virtual ~MstStorage() = default;
  };
} // namespace iroha
#endif //IROHA_MST_STORAGE_HPP
