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

  template <typename Subject>
  void shareState(ConstRefState state, Subject &subject) {
    if (not state.isEmpty()) {
      auto completed_transactions = state.getTransactions();
      std::for_each(
          completed_transactions.begin(), completed_transactions.end(),
          [&subject](const auto tx) { subject.get_subscriber().on_next(tx); });
    }
  }

  FairMstProcessor::FairMstProcessor(
      std::shared_ptr<iroha::network::MstTransport> transport,
      std::shared_ptr<MstStorage> storage,
      std::shared_ptr<PropagationStrategy> strategy,
      std::shared_ptr<MstTimeProvider> time_provider)
      : MstProcessor(),
        transport_(std::move(transport)),
        storage_(std::move(storage)),
        strategy_(std::move(strategy)),
        time_provider_(std::move(time_provider)) {
    strategy_->emitter().subscribe(
        [this](auto data) { this->onPropagate(data); });
    log_ = logger::log("FairMstProcessor");
  }

  // -------------------------| MstProcessor override |-------------------------

  auto FairMstProcessor::propagateTransactionImpl(
      ConstRefTransaction transaction)
      -> decltype(propagateTransaction(transaction)) {
    shareState(storage_->updateOwnState(transaction), transactions_subject_);
    shareState(
        storage_->getExpiredTransactions(time_provider_->getCurrentTime()),
        expired_subject_);
  }

  auto FairMstProcessor::onStateUpdateImpl() const
      -> decltype(onStateUpdate()) {
    return state_subject_.get_observable();
  }

  auto FairMstProcessor::onPreparedTransactionsImpl() const
      -> decltype(onPreparedTransactions()) {
    return transactions_subject_.get_observable();
  }

  auto FairMstProcessor::onExpiredTransactionsImpl() const
      -> decltype(onExpiredTransactions()) {
    return expired_subject_.get_observable();
  }

  // -------------------| MstTransportNotification override |-------------------

  void FairMstProcessor::onNewState(ConstRefPeer from,
                                    ConstRefState new_state) {
    auto current_time = time_provider_->getCurrentTime();

    // update state
    // todo wrap in method
    auto new_transactions =
        std::make_shared<MstState>(storage_->whatsNew(new_state));
    state_subject_.get_subscriber().on_next(new_transactions);

    // completed transactions
    shareState(storage_->apply(from, new_state), transactions_subject_);

    // expired transactions
    auto expired_transactions = storage_->getDiffState(from, current_time);
    shareState(expired_transactions, this->expired_subject_);
  }

  // -----------------------------| private api |-----------------------------

  void FairMstProcessor::onPropagate(
      const PropagationStrategy::PropagationData &data) {
    auto current_time = time_provider_->getCurrentTime();
    std::for_each(data.begin(), data.end(),
                  [this, &current_time](const auto &peer) {
                    transport_->sendState(
                        peer, storage_->getDiffState(peer, current_time));
                  });
  }

}  // namespace iroha
