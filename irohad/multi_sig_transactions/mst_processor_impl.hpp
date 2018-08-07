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

    ~FairMstProcessor();

    // ------------------------| MstProcessor override |------------------------

    auto propagateBatchImpl(const DataType &batch)
        -> decltype(propagateBatch(batch)) override;

    auto onStateUpdateImpl() const -> decltype(onStateUpdate()) override;

    auto onPreparedBatchesImpl() const
        -> decltype(onPreparedBatches()) override;

    auto onExpiredBatchesImpl() const -> decltype(onExpiredBatches()) override;

    // ------------------| MstTransportNotification override |------------------

    void onNewState(const std::shared_ptr<shared_model::interface::Peer> &from,
                    ConstRefState new_state) override;

    // ----------------------------| end override |-----------------------------

   private:
    // -----------------------------| private api |-----------------------------

    /**
     * Invoke when propagation strategy emit new data
     * @param data - propagated data
     */
    void onPropagate(const PropagationStrategy::PropagationData &data);

    // -------------------------------| fields |--------------------------------
    std::shared_ptr<iroha::network::MstTransport> transport_;
    std::shared_ptr<MstStorage> storage_;
    std::shared_ptr<PropagationStrategy> strategy_;
    std::shared_ptr<MstTimeProvider> time_provider_;

    // rx subjects

    /// use for share new states from other peers
    rxcpp::subjects::subject<std::shared_ptr<MstState>> state_subject_;

    /// use for share completed batches
    rxcpp::subjects::subject<DataType> batches_subject_;

    /// use for share expired batches
    rxcpp::subjects::subject<DataType> expired_subject_;

    /// use for tracking the propagation subscription
    rxcpp::composite_subscription propagation_subscriber_;
  };
}  // namespace iroha

#endif  // IROHA_MST_PROCESSOR_IMPL_HPP
