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
#include "multi_sig_transactions/mst_types.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"

namespace iroha {

  /**
   * MstStorage responsible for manage own and others MstStates.
   */
  class MstStorage {
   public:
    // ------------------------------| user API |-------------------------------

    /**
     * Apply new state for peer
     * @param target_peer - key for for updating state
     * @param new_state - state with new data
     * @return State with completed transaction
     * General note: implementation of method covered by lock
     */
    MstState apply(ConstPeer &target_peer, const MstState &new_state);

    /**
     * Provide updating state of current peer with new transaction
     * @param tx - new transaction for insertion in state
     * @return State with completed transaction
     * General note: implementation of method covered by lock
     */
    MstState updateOwnState(const TransactionType &tx);

    /**
     * Remove expired transactions and return them
     * @return State with expired transactions
     * General note: implementation of method covered by lock
     */
    MstState getExpiredTransactions(const TimeType &current_time);

    /**
     * Make state based on diff of own and target states.
     * All expired transactions will be removed from diff.
     * @return difference between own and target state
     * General note: implementation of method covered by lock
     */
    MstState getDiffState(ConstPeer &target_peer, const TimeType &current_time);

    virtual ~MstStorage() = default;

   protected:
    // ------------------------------| class API |------------------------------

    /**
     * Constructor provide initialization of protected fields, such as logger.
     */
    MstStorage();

   private:
    virtual auto applyImpl(ConstPeer &target_peer, const MstState &new_state)
        -> decltype(apply(target_peer, new_state)) = 0;

    virtual auto updateOwnStateImpl(const TransactionType &tx)
        -> decltype(updateOwnState(tx)) = 0;

    virtual auto getExpiredTransactionsImpl(const TimeType &current_time)
        -> decltype(getExpiredTransactions(current_time)) = 0;

    virtual auto getDiffStateImpl(ConstPeer &target_peer,
                                  const TimeType &current_time)
        -> decltype(getDiffState(target_peer, current_time)) = 0;

    // -------------------------------| fields |--------------------------------

    mutable std::mutex mutex_;

   protected:
    logger::Logger log_;
  };
}  // namespace iroha
#endif  // IROHA_MST_STORAGE_HPP
