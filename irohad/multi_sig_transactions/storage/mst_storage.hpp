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

#include <mutex>
#include "logger/logger.hpp"
#include "model/peer.hpp"
#include "multi_sig_transactions/storage/mst_state.hpp"
#include "multi_sig_transactions/mst_types.hpp"

namespace iroha {

  /**
   * MstStorage responsible for manage own and others MstStates.
   */
  class MstStorage {
   public:
// --------------------------------| user API |---------------------------------

    /**
     * Constructor provide initialization of protected fields, such as logger.
     */
    MstStorage();

    /**
     * Apply new state for peer
     * @param target_peer - key for for updating state
     * @param new_state - state with new data
     * General note: implementation of method covered by lock
     */
    void apply(ConstPeer &target_peer, MstState new_state);

    /**
     * Provide updating state of current peer with new transaction
     * @param tx - new transaction for insertion in state
     * General note: implementation of method covered by lock
     */
    void updateOwnState(TransactionType tx);

    /**
     * Return difference between own and target state
     * General note: implementation of method covered by lock
     */
    MstState getDiffState(ConstPeer &target_peer) const;

    virtual ~MstStorage() = default;
   private:
// --------------------------------| class API |--------------------------------

    virtual auto applyImpl(ConstPeer &target_peer, MstState &new_state)
    -> decltype(apply(target_peer, new_state)) = 0;

    virtual auto updateOwnStateImpl(TransactionType tx)
    -> decltype(updateOwnState(tx)) = 0;

    virtual MstState getDiffStateImpl(ConstPeer &target_peer) const = 0;

// ---------------------------------| fields |----------------------------------

    mutable std::mutex mutex_;

   protected:
    logger::Logger log_;
  };
} // namespace iroha
#endif //IROHA_MST_STORAGE_HPP
