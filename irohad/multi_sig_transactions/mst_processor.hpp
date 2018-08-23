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
     * Propagate batch in network for signing by other
     * participants
     * @param transaction - transaction for propagation
     */
    void propagateBatch(const DataType &batch);

    /**
     * Prove updating of state for handling status of signing
     */
    rxcpp::observable<std::shared_ptr<MstState>> onStateUpdate() const;

    /**
     * Observable emit batches which are prepared for further processing in
     * system
     */
    rxcpp::observable<DataType> onPreparedBatches() const;

    /**
     * Observable emit expired by time transactions
     */
    rxcpp::observable<DataType> onExpiredBatches() const;

    virtual ~MstProcessor() = default;

   protected:
    MstProcessor();

    logger::Logger log_;

   private:
    // ------------------------| inheritance interface |------------------------

    /**
     * @see propagateTransaction method
     */
    virtual auto propagateBatchImpl(const DataType &batch)
        -> decltype(propagateBatch(batch)) = 0;

    /**
     * @see onStateUpdate method
     */
    virtual auto onStateUpdateImpl() const -> decltype(onStateUpdate()) = 0;

    /**
     * @see onPreparedTransactions method
     */
    virtual auto onPreparedBatchesImpl() const
        -> decltype(onPreparedBatches()) = 0;

    /**
     * @see onExpiredTransactions method
     */
    virtual auto onExpiredBatchesImpl() const
        -> decltype(onExpiredBatches()) = 0;
  };
}  // namespace iroha

#endif  // IROHA_MST_PROPAGATOR_HPP
