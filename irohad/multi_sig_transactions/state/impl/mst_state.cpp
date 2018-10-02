/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/state/mst_state.hpp"

#include <boost/range/algorithm/find.hpp>
#include <boost/range/combine.hpp>
#include <utility>

#include "common/set.hpp"

namespace iroha {

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(const CompleterType &completer) {
    return MstState(completer);
  }

  StateUpdateResult MstState::operator+=(const DataType &rhs) {
    auto state_update = StateUpdateResult{
        std::make_shared<MstState>(MstState::empty(completer_)),
        std::make_shared<MstState>(MstState::empty(completer_))};
    insertOne(state_update, rhs);
    return state_update;
  }

  StateUpdateResult MstState::operator+=(const MstState &rhs) {
    auto state_update = StateUpdateResult{
        std::make_shared<MstState>(MstState::empty(completer_)),
        std::make_shared<MstState>(MstState::empty(completer_))};
    for (auto &&rhs_tx : rhs.internal_state_) {
      insertOne(state_update, rhs_tx);
    }
    return state_update;
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(this->completer_,
                    set_difference(this->internal_state_, rhs.internal_state_));
  }

  bool MstState::operator==(const MstState &rhs) const {
    return std::all_of(
        internal_state_.begin(), internal_state_.end(), [&rhs](auto &i) {
          return rhs.internal_state_.find(i) != rhs.internal_state_.end();
        });
  }

  bool MstState::isEmpty() const {
    return internal_state_.empty();
  }

  std::unordered_set<DataType,
                     iroha::model::PointerBatchHasher<DataType>,
                     BatchHashEquality>
  MstState::getBatches() const {
    return {internal_state_.begin(), internal_state_.end()};
  }

  MstState MstState::eraseByTime(const TimeType &time) {
    MstState out = MstState::empty(completer_);
    while (not index_.empty() and (*completer_)(index_.top(), time)) {
      auto iter = internal_state_.find(index_.top());

      out += *iter;
      internal_state_.erase(iter);
      index_.pop();
    }
    return out;
  }

  // ------------------------------| private api |------------------------------

  /**
   * Merge signatures in batches
   * @param target - batch for inserting
   * @param donor - batch with transactions to copy signatures from
   * @return return if at least one new signature was inserted
   */
  bool mergeSignaturesInBatch(DataType &target, const DataType &donor) {
    auto inserted_new_signatures = false;
    for (auto zip :
         boost::combine(target->transactions(), donor->transactions())) {
      const auto &target_tx = zip.get<0>();
      const auto &donor_tx = zip.get<1>();
      inserted_new_signatures = std::accumulate(
          std::begin(donor_tx->signatures()),
          std::end(donor_tx->signatures()),
          inserted_new_signatures,
          [&target_tx](bool accumulator, const auto &signature) {
            return target_tx->addSignature(signature.signedData(),
                                           signature.publicKey())
                or accumulator;
          });
    }
    return inserted_new_signatures;
  }

  MstState::MstState(const CompleterType &completer)
      : MstState(completer, InternalStateType{}) {}

  MstState::MstState(const CompleterType &completer,
                     const InternalStateType &transactions)
      : completer_(completer),
        internal_state_(transactions.begin(), transactions.end()),
        index_(transactions.begin(), transactions.end()) {
    log_ = logger::log("MstState");
  }

  void MstState::insertOne(StateUpdateResult &state_update,
                           const DataType &rhs_batch) {
    log_->info("batch: {}", rhs_batch->toString());
    auto corresponding = internal_state_.find(rhs_batch);
    if (corresponding == internal_state_.end()) {
      // when state does not contain transaction
      rawInsert(rhs_batch);
      state_update.updated_state_->rawInsert(rhs_batch);
      return;
    }

    DataType found = *corresponding;
    // Append new signatures to the existing state
    auto inserted_new_signatures = mergeSignaturesInBatch(found, rhs_batch);

    if ((*completer_)(found)) {
      // state already has completed transaction,
      // remove from state and return it
      internal_state_.erase(internal_state_.find(found));
      state_update.completed_state_->rawInsert(found);
      return;
    }

    // if batch still isn't completed, return it, if new signatures were
    // inserted
    if (inserted_new_signatures) {
      state_update.updated_state_->rawInsert(found);
    }
  }

  void MstState::rawInsert(const DataType &rhs_batch) {
    internal_state_.insert(rhs_batch);
    index_.push(rhs_batch);
  }

}  // namespace iroha
