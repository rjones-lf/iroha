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
    auto state_update = storage_->updateOwnState(batch);
    completedBatchesNotify(*state_update.completed_state_);
    updatedBatchesNotify(*state_update.updated_state_);
    expiredBatchesNotify(
        storage_->getExpiredTransactions(time_provider_->getCurrentTime()));
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

  // TODO [IR-1687] Akvinikym 10.09.18: three methods below should be one
  void FairMstProcessor::completedBatchesNotify(ConstRefState state) const {
    if (not state.isEmpty()) {
      auto completed_batches = state.getBatches();
      std::for_each(completed_batches.begin(),
                    completed_batches.end(),
                    [this](const auto &batch) {
                      batches_subject_.get_subscriber().on_next(batch);
                    });
    }
  }

  void FairMstProcessor::updatedBatchesNotify(ConstRefState state) const {
    if (not state.isEmpty()) {
      state_subject_.get_subscriber().on_next(
          std::make_shared<MstState>(state));
    }
  }

  void FairMstProcessor::expiredBatchesNotify(ConstRefState state) const {
    if (not state.isEmpty()) {
      auto expired_batches = state.getBatches();
      std::for_each(expired_batches.begin(),
                    expired_batches.end(),
                    [this](const auto &batch) {
                      expired_subject_.get_subscriber().on_next(batch);
                    });
    }
  }

  // -------------------| MstTransportNotification override |-------------------

  void FairMstProcessor::onNewState(
      const std::shared_ptr<shared_model::interface::Peer> &from,
      ConstRefState new_state) {
    log_->info("Applying new state");
    auto current_time = time_provider_->getCurrentTime();

    auto state_update = storage_->apply(from, new_state);

    // updated batches
    updatedBatchesNotify(*state_update.updated_state_);
    log_->info("New batches size: {}",
               state_update.updated_state_->getBatches().size());

    // completed batches
    completedBatchesNotify(*state_update.completed_state_);

    // expired batches
    expiredBatchesNotify(storage_->getDiffState(from, current_time));
  }

  // -----------------------------| private api |-----------------------------

  void FairMstProcessor::onPropagate(
      const PropagationStrategy::PropagationData &data) {
    auto current_time = time_provider_->getCurrentTime();
    auto size = data.size();
    std::for_each(data.begin(),
                  data.end(),
                  [this, &current_time, size](const auto &peer) {
                    auto diff = storage_->getDiffState(peer, current_time);
                    if (not diff.isEmpty()) {
                      log_->info("Propagate new data[{}]", size);
                      transport_->sendState(*peer, diff);
                    }
                  });
  }

}  // namespace iroha
