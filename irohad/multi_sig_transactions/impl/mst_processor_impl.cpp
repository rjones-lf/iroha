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
      auto completed_batches = state.getBatches();
      std::for_each(completed_batches.begin(),
                    completed_batches.end(),
                    [&subject](const auto &batch) {
                      subject.get_subscriber().on_next(batch);
                    });
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
        time_provider_(std::move(time_provider)),
        propagation_subscriber_(strategy_->emitter().subscribe(
            [this](auto data) { this->onPropagate(data); })) {
    log_ = logger::log("FairMstProcessor");
  }

  FairMstProcessor::~FairMstProcessor() {
    propagation_subscriber_.unsubscribe();
  }

  // -------------------------| MstProcessor override |-------------------------

  auto FairMstProcessor::propagateBatchImpl(const iroha::DataType &batch)
      -> decltype(propagateBatch(batch)) {
    shareState(storage_->updateOwnState(batch), batches_subject_);
    shareState(
        storage_->getExpiredTransactions(time_provider_->getCurrentTime()),
        expired_subject_);
  }

  auto FairMstProcessor::onStateUpdateImpl() const
      -> decltype(onStateUpdate()) {
    return state_subject_.get_observable();
  }

  auto FairMstProcessor::onPreparedBatchesImpl() const
      -> decltype(onPreparedBatches()) {
    return batches_subject_.get_observable();
  }

  auto FairMstProcessor::onExpiredBatchesImpl() const
      -> decltype(onExpiredBatches()) {
    return expired_subject_.get_observable();
  }

  // -------------------| MstTransportNotification override |-------------------

  void FairMstProcessor::onNewState(
      const std::shared_ptr<shared_model::interface::Peer> &from,
      ConstRefState new_state) {
    log_->info("Applying new state");
    auto current_time = time_provider_->getCurrentTime();

    // update state
    auto new_batches =
        std::make_shared<MstState>(storage_->whatsNew(new_state));
    state_subject_.get_subscriber().on_next(new_batches);

    log_->info("New batches size: {}", new_batches->getBatches().size());
    // completed batches
    shareState(storage_->apply(from, new_state), batches_subject_);

    // expired batches
    auto expired_batches = storage_->getDiffState(from, current_time);
    shareState(expired_batches, this->expired_subject_);
  }

  // -----------------------------| private api |-----------------------------

  void FairMstProcessor::onPropagate(
      const PropagationStrategy::PropagationData &data) {
    log_->info("Propagate new data[{}]", data.size());
    auto current_time = time_provider_->getCurrentTime();
    std::for_each(
        data.begin(), data.end(), [this, &current_time](const auto &peer) {
          auto diff = storage_->getDiffState(peer, current_time);
          if (not diff.isEmpty()) {
            transport_->sendState(*peer, diff);
          }
        });
  }

}  // namespace iroha
