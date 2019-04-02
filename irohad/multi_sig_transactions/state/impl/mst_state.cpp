/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/state/mst_state.hpp"

#include <algorithm>
#include <utility>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/combine.hpp>
#include "common/set.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"
#include "logger/logger.hpp"

namespace {
  shared_model::interface::types::TimestampType oldestTimestamp(
      const iroha::BatchPtr &batch) {
    auto timestamps =
        batch->transactions()
        | boost::adaptors::transformed(
              +[](const std::shared_ptr<shared_model::interface::Transaction>
                      &tx) { return tx->createdTime(); });
    const auto min_it = std::min_element(timestamps.begin(), timestamps.end());
    assert(min_it != timestamps.end());
    return min_it == timestamps.end() ? 0 : *min_it;
  }
}  // namespace

namespace iroha {

  bool BatchHashEquality::operator()(const DataType &left_tx,
                                     const DataType &right_tx) const {
    return left_tx->reducedHash() == right_tx->reducedHash();
  }

  DefaultCompleter::DefaultCompleter(std::chrono::minutes expiration_time)
      : expiration_time_(expiration_time) {}

  bool DefaultCompleter::operator()(const DataType &batch) const {
    return std::all_of(batch->transactions().begin(),
                       batch->transactions().end(),
                       [](const auto &tx) {
                         return boost::size(tx->signatures()) >= tx->quorum();
                       });
  }

  bool DefaultCompleter::operator()(const DataType &batch,
                                    const TimeType &time) const {
    return oldestTimestamp(batch)
        + expiration_time_ / std::chrono::milliseconds(1)
        < time;
  }

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(logger::LoggerPtr log,
                           const CompleterType &completer) {
    return MstState(completer, std::move(log));
  }

  StateUpdateResult MstState::operator+=(const DataType &rhs) {
    auto state_update = StateUpdateResult{
        std::make_shared<MstState>(MstState::empty(log_, completer_)),
        std::make_shared<MstState>(MstState::empty(log_, completer_))};
    insertOne(state_update, rhs);
    return state_update;
  }

  StateUpdateResult MstState::operator+=(const MstState &rhs) {
    auto state_update = StateUpdateResult{
        std::make_shared<MstState>(MstState::empty(log_, completer_)),
        std::make_shared<MstState>(MstState::empty(log_, completer_))};
    for (auto &&rhs_tx : rhs.internal_state_) {
      insertOne(state_update, rhs_tx);
    }
    return state_update;
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(this->completer_,
                    set_difference(this->internal_state_, rhs.internal_state_),
                    log_);
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
                     iroha::model::PointerBatchHasher,
                     BatchHashEquality>
  MstState::getBatches() const {
    return {internal_state_.begin(), internal_state_.end()};
  }

  MstState MstState::eraseByTime(const TimeType &time) {
    MstState out = MstState::empty(log_, completer_);
    for (auto it = index_.left.begin();
         it != index_.left.end() and (*completer_)(it->second, time);) {
      out += it->second;
      internal_state_.erase(it->second);
      it = index_.left.erase(it);
    }
    return out;
  }

  // ------------------------------| private api |------------------------------

  bool MstState::Less::operator()(const DataType &left,
                                  const DataType &right) const {
    return oldestTimestamp(left) < oldestTimestamp(right);
  }

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

  MstState::MstState(const CompleterType &completer, logger::LoggerPtr log)
      : MstState(completer, InternalStateType{}, std::move(log)) {}

  MstState::MstState(const CompleterType &completer,
                     const InternalStateType &batches,
                     logger::LoggerPtr log)
      : completer_(completer),
        internal_state_(batches.begin(), batches.end()),
        log_(std::move(log)) {
          for (const auto &batch : batches) {
            index_.insert(IndexType::value_type(oldestTimestamp(batch), batch));
          }
        }

  void MstState::insertOne(StateUpdateResult &state_update,
                           const DataType &rhs_batch) {
    log_->info("batch: {}", *rhs_batch);
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
    index_.insert(IndexType::value_type(oldestTimestamp(rhs_batch), rhs_batch));
  }

  bool MstState::contains(const DataType &element) const {
    return internal_state_.find(element) != internal_state_.end();
  }

}  // namespace iroha
