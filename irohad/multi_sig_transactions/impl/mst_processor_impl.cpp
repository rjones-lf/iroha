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

#include <utility>

#include "multi_sig_transactions/mst_processor_impl.hpp"

namespace iroha {
  FairMstProcessor::FairMstProcessor(
      shp<iroha::network::MstTransport> transport,
      shp<MstStorage> storage,
      shp<PropagationStrategy<PropagationDataType>> strategy)
      : MstProcessor(),
        transport_(std::move(transport)),
        storage_(std::move(storage)),
        strategy_(std::move(strategy)) {
  }

// --------------------------| MstProcessor override |--------------------------

  void FairMstProcessor::propagateTransactionImpl(shp<model::Transaction> transaction) {
    storage_->updateOwnState(transaction);
  }

  rxcpp::observable<shp<MstState>> FairMstProcessor::onStateUpdateImpl() const {
    return state_subject_.get_observable();
  }

  rxcpp::observable<shp<model::Transaction>> FairMstProcessor::onPreparedTransactionsImpl() const {
    return transactions_subject_.get_observable();
  }

// --------------------| MstTransportNotification override |--------------------

  void FairMstProcessor::onStateUpdate(ConstPeer from, MstState new_state) {
//    auto diff = storage_->apply(from, std::move(new_state));


  }
} // namespace iroha
