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

#ifndef IROHA_MST_PROCESSOR_IMPL_HPP
#define IROHA_MST_PROCESSOR_IMPL_HPP

#include <memory>
#include "logger/logger.hpp"
#include "multi_sig_transactions/mst_processor.hpp"
#include "multi_sig_transactions/mst_propagation_strategy.hpp"
#include "multi_sig_transactions/mst_time_provider.hpp"
#include "multi_sig_transactions/storage/mst_storage.hpp"
#include "network/mst_transport.hpp"

namespace iroha {

  /**
   * Effective implementation of MstProcessor,
   * that implements gossip propagation of own state
   */
  class FairMstProcessor : public MstProcessor,
                           public iroha::network::MstTransportNotification {
   public:
    /**
     * @param transport - connection to other peers in network
     * @param storage  - repository for storing states
     * @param strategy - propagation mechanism for sharing state with others
     * @param time_provider - repository of current time
     */
    FairMstProcessor(std::shared_ptr<iroha::network::MstTransport> transport,
                     std::shared_ptr<MstStorage> storage,
                     std::shared_ptr<PropagationStrategy> strategy,
                     std::shared_ptr<MstTimeProvider> time_provider);

    // ------------------------| MstProcessor override |------------------------

    auto propagateTransactionImpl(ConstRefTransaction transaction)
        -> decltype(propagateTransaction(transaction)) override;

    auto onStateUpdateImpl() const -> decltype(onStateUpdate()) override;

    auto onPreparedTransactionsImpl() const
        -> decltype(onPreparedTransactions()) override;

    auto onExpiredTransactionsImpl() const
        -> decltype(onExpiredTransactions()) override;

    // ------------------| MstTransportNotification override |------------------

    void onNewState(ConstRefPeer from, ConstRefState new_state) override;

    // ----------------------------| end override |-----------------------------

    virtual ~FairMstProcessor() = default;

   private:
    // -----------------------------| private api |-----------------------------

    /**
     * Invoke when propagation strategy emit new data
     * @param data - propagated data
     */
    void onPropagate(const PropagationStrategy::PropagationData& data);

    // -------------------------------| fields |--------------------------------
    std::shared_ptr<iroha::network::MstTransport> transport_;
    std::shared_ptr<MstStorage> storage_;
    std::shared_ptr<PropagationStrategy> strategy_;
    std::shared_ptr<MstTimeProvider> time_provider_;

    logger::Logger log_;

    // rx subjects

    /// use for share new states from other peers
    rxcpp::subjects::subject<std::shared_ptr<MstState>> state_subject_;

    /// use for share completed transactions
    rxcpp::subjects::subject<std::shared_ptr<model::Transaction>>
        transactions_subject_;

    /// use for share expired transactions
    rxcpp::subjects::subject<std::shared_ptr<model::Transaction>>
        expired_subject_;
  };
}  // namespace iroha

#endif  // IROHA_MST_PROCESSOR_IMPL_HPP
