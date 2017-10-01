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

#include "multi_sig_transactions/mst_processor.hpp"
#include "multi_sig_transactions/mst_propagation_strategy.hpp"
#include "multi_sig_transactions/storage/mst_storage.hpp"
#include "network/mst_transport.hpp"
#include "memory"
#include "vector"

namespace iroha {

  class FairMstProcessor : public MstProcessor,
                           public iroha::network::MstTransportNotification {
   public:
    using PropagationDataType = std::vector<ConstPeer>;

    explicit FairMstProcessor(shp<iroha::network::MstTransport> transport,
                              shp<MstStorage> storage,
                              shp<PropagationStrategy<PropagationDataType>> strategy);

// --------------------------| MstProcessor override |--------------------------

    void propagateTransactionImpl(shp<model::Transaction> transaction) override;

    rxcpp::observable<shp<MstState>> onStateUpdateImpl() const override;

    rxcpp::observable<shp<model::Transaction>> onPreparedTransactionsImpl() const override;

// --------------------| MstTransportNotification override |--------------------

    void onStateUpdate(ConstPeer from, MstState new_state) override;

// ------------------------------| end override |-------------------------------

    virtual ~FairMstProcessor() = default;

   private:
// ---------------------------------| fields |----------------------------------


  };
} // namespace iroha

#endif //IROHA_MST_PROCESSOR_IMPL_HPP
