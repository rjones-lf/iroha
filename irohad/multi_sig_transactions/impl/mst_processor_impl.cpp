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
      std::shared_ptr<iroha::network::MstTransport> transport,
      std::shared_ptr<MstStorage> storage,
      std::shared_ptr<PropagationStrategy> strategy)
      : MstProcessor(),
        transport_(transport),
        storage_(storage),
        strategy_(strategy) {}

  // -------------------------| MstProcessor override |-------------------------

  void FairMstProcessor::propagateTransactionImpl(
      ConstRefTransaction transaction) {
    storage_->updateOwnState(transaction);
  }

  rxcpp::observable<std::shared_ptr<MstState>>
  FairMstProcessor::onStateUpdateImpl() const {
    return state_subject_.get_observable();
  }

  rxcpp::observable<std::shared_ptr<model::Transaction>>
  FairMstProcessor::onPreparedTransactionsImpl() const {
    return transactions_subject_.get_observable();
  }

  // -------------------| MstTransportNotification override |-------------------

  void FairMstProcessor::onStateUpdate(ConstRefPeer from,
                                       ConstRefState new_state) {
    //    auto diff = storage_->apply(from, std::move(new_state));
  }
}  // namespace iroha
