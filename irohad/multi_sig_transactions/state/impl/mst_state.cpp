/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/state/mst_state.hpp"

#include <utility>

#include <boost/range/algorithm/find.hpp>
#include <boost/range/combine.hpp>
#include "common/set.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"
#include "logger/logger.hpp"

template <class T>
static size_t countTxsInBatches(const T &batches) {
  size_t size = 0;
  for (const auto &batch : batches) {
    size += boost::size(batch->transactions());
  }
  return size;
}

namespace iroha {

  bool BatchHashEquality::operator()(const DataType &left_tx,
                                     const DataType &right_tx) const {
    return left_tx->reducedHash() == right_tx->reducedHash();
  }

  DefaultCompleter::DefaultCompleter(std::chrono::minutes expiration_time)
      : expiration_time_(expiration_time) {}

  bool DefaultCompleter::isCompleted(const DataType &batch) const {
    return std::all_of(batch->transactions().begin(),
                       batch->transactions().end(),
                       [](const auto &tx) {
                         return boost::size(tx->signatures()) >= tx->quorum();
                       });
  }

  bool DefaultCompleter::isExpired(const DataType &batch,
                                   const TimeType &current_time) const {
    return std::any_of(batch->transactions().begin(),
                       batch->transactions().end(),
                       [&](const auto &tx) {
                         return tx->createdTime()
                             + expiration_time_ / std::chrono::milliseconds(1)
                             < current_time;
                       });
  }

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(const CompleterType &completer,
                           size_t transaction_limit,
                           logger::LoggerPtr log) {
    return MstState(completer, transaction_limit, std::move(log));
  }

  StateUpdateResult MstState::operator+=(const DataType &rhs) {
    auto state_update =
        StateUpdateResult{std::make_shared<MstState>(
                              MstState::empty(completer_, txs_limit_, log_)),
                          std::make_shared<MstState>(
                              MstState::empty(completer_, txs_limit_, log_))};
    insertOne(state_update, rhs);
    return state_update;
  }

  StateUpdateResult MstState::operator+=(const MstState &rhs) {
    auto state_update =
        StateUpdateResult{std::make_shared<MstState>(
                              MstState::empty(completer_, txs_limit_, log_)),
                          std::make_shared<MstState>(
                              MstState::empty(completer_, txs_limit_, log_))};
    for (auto &&rhs_tx : rhs.internal_state_) {
      insertOne(state_update, rhs_tx);
    }
    return state_update;
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(this->completer_,
                    txs_limit_,
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

  MstState MstState::extractExpired(const TimeType &current_time) {
    MstState out = MstState::empty(completer_, txs_limit_, log_);
    extractExpiredImpl(current_time, out);
    return out;
  }

  void MstState::eraseExpired(const TimeType &current_time) {
    extractExpiredImpl(current_time, boost::none);
  }

  size_t MstState::transactionsQuantity() const {
    assert(txs_quantity_ == countTxsInBatches(internal_state_));
    return txs_quantity_;
  }

  size_t MstState::batchesQuantity() const {
    return internal_state_.size();
  }

  // ------------------------------| private api |------------------------------

  bool MstState::Less::operator()(const DataType &left,
                                  const DataType &right) const {
    return left->transactions().at(0)->createdTime()
        < right->transactions().at(0)->createdTime();
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

  MstState::MstState(const CompleterType &completer,
                     size_t transaction_limit,
                     logger::LoggerPtr log)
      : MstState(
            completer, transaction_limit, InternalStateType{}, std::move(log)) {
  }

  MstState::MstState(const CompleterType &completer,
                     size_t transaction_limit,
                     const InternalStateType &batches,
                     logger::LoggerPtr log)
      : completer_(completer),
        internal_state_(batches.begin(), batches.end()),
        index_(batches.begin(), batches.end()),
        txs_limit_(transaction_limit),
        txs_quantity_(countTxsInBatches(batches)),
        log_(std::move(log)) {}

  void MstState::insertOne(StateUpdateResult &state_update,
                           const DataType &rhs_batch) {
    log_->info("batch: {}", *rhs_batch);
    auto corresponding = internal_state_.find(rhs_batch);
    if (corresponding == internal_state_.end()) {
      // when state does not contain transaction
      if (transactionsQuantity() + boost::size(rhs_batch->transactions())
          <= txs_limit_) {
        // there is enough room for the new batch
        rawInsert(rhs_batch);
        state_update.updated_state_->rawInsert(rhs_batch);
      } else {
        // there is not enough room for the new batch
        log_->info(
            "Dropped a batch because it would exceed the transaction limit "
            "(currently have {} out of {} transactions in state): {}",
            transactionsQuantity(),
            txs_limit_,
            *rhs_batch);
      }
      return;
    }

    DataType found = *corresponding;
    // Append new signatures to the existing state
    auto inserted_new_signatures = mergeSignaturesInBatch(found, rhs_batch);

    if (completer_->isCompleted(found)) {
      // state already has completed transaction,
      // remove from state and return it
      assert(txs_quantity_ >= boost::size(found->transactions()));
      txs_quantity_ -= boost::size(found->transactions());
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
    txs_quantity_ += boost::size(rhs_batch->transactions());
    index_.push(rhs_batch);
  }

  bool MstState::contains(const DataType &element) const {
    return internal_state_.find(element) != internal_state_.end();
  }

  void MstState::extractExpiredImpl(const TimeType &current_time,
                                    boost::optional<MstState &> extracted) {
    while (not index_.empty()
           and completer_->isExpired(index_.top(), current_time)) {
      auto iter = internal_state_.find(index_.top());
      assert(iter != internal_state_.end());

      if(extracted) {
        *extracted += *iter;
      }
      internal_state_.erase(iter);
      index_.pop();
    }
  }

}  // namespace iroha
