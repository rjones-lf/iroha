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

#ifndef IROHA_MST_PROPAGATOR_HPP
#define IROHA_MST_PROPAGATOR_HPP

#include <memory>
#include <mutex>
#include <rxcpp/rx.hpp>
#include "logger/logger.hpp"
#include "model/transaction.hpp"
#include "multi_sig_transactions/mst_types.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"

namespace iroha {

  /**
   * MstProcessor is responsible for organization of sharing multi-signature
   * transactions in network
   */
  class MstProcessor {
   public:
    // ---------------------------| user interface |----------------------------

    /**
     * Propagate in network multi-signature transaction for signing by other
     * participants
     * @param transaction - transaction for propagation
     * General note: implementation of method covered by lock
     */
    void propagateTransaction(ConstRefTransaction transaction);

    /**
     * Prove updating of state for handling status of signing
     * General note: implementation of method covered by lock
     */
    rxcpp::observable<std::shared_ptr<MstState>> onStateUpdate() const;

    /**
     * Observable emit transactions that prepared to processing in system
     * General note: implementation of method covered by lock
     */
    rxcpp::observable<TransactionType> onPreparedTransactions() const;

    /**
     * Observable emit expired by time transactions
     * General note: implementation of method covered by lock
     */
    rxcpp::observable<TransactionType> onExpiredTransactions() const;

    virtual ~MstProcessor() = default;

   protected:
    MstProcessor();

   private:
    // ------------------------| inheritance interface |------------------------

    /**
     * @see propagateTransaction method
     */
    virtual auto propagateTransactionImpl(ConstRefTransaction transaction)
        -> decltype(propagateTransaction(transaction)) = 0;

    /**
     * @see onStateUpdate method
     */
    virtual auto onStateUpdateImpl() const -> decltype(onStateUpdate()) = 0;

    /**
     * @see onPreparedTransactions method
     */
    virtual auto onPreparedTransactionsImpl() const
        -> decltype(onPreparedTransactions()) = 0;

    /**
     * @see onExpiredTransactions method
     */
    virtual auto onExpiredTransactionsImpl() const
        -> decltype(onExpiredTransactions()) = 0;

    // -------------------------------| fields |--------------------------------

    std::mutex mutex_;

    logger::Logger log_;
  };
}  // namespace iroha

#endif  // IROHA_MST_PROPAGATOR_HPP
