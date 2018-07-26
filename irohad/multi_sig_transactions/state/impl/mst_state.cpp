/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/state/mst_state.hpp"

#include <boost/range/algorithm/find.hpp>
#include <utility>

#include "backend/protobuf/transaction.hpp"
#include "common/set.hpp"

namespace iroha {

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(const CompleterType &completer) {
    return MstState(completer);
  }

  MstState MstState::operator+=(const DataType &rhs) {
    auto result = MstState::empty(completer_);
    insertOne(result, rhs);
    return result;
  }

  MstState MstState::operator+=(const MstState &rhs) {
    auto result = MstState::empty(completer_);
    for (auto &&rhs_tx : rhs.internal_state_) {
      insertOne(result, rhs_tx);
    }
    return result;
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(this->completer_,
                    set_difference(this->internal_state_, rhs.internal_state_));
  }

  bool MstState::operator==(const MstState &rhs) const {
    return std::is_permutation(
        internal_state_.begin(),
        internal_state_.end(),
        rhs.internal_state_.begin(),
        [](auto &lhs, auto &rhs) { return *lhs == *rhs; });
  }

  bool MstState::isEmpty() const {
    return internal_state_.empty();
  }

  std::vector<DataType> MstState::getBatches() const {
    std::vector<DataType> result;
    std::for_each(internal_state_.begin(),
                  internal_state_.end(),
                  [&result](const auto &val) { result.push_back(val); });
    return result;
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
   * @param donor - batch with interested transactions
   * @return false if hashes of transactions don't satisfy each other
   */
  bool mergeSignaturesInBatch(DataType &target, const DataType &donor) {
    if (target->reducedHash() != donor->reducedHash()) {
      return false;
    }

    for (auto &target_tx : target->transactions()) {
      for (auto &donor_tx : donor->transactions()) {
        if (target_tx->hash() == donor_tx->hash()) {
          std::for_each(donor_tx->signatures().begin(),
                        donor_tx->signatures().end(),
                        [&target_tx](const auto &signature) {
                          target_tx->addSignature(signature.signedData(),
                                                  signature.publicKey());
                        });
        } else {
          return false;
        }
      }
    }
    return true;
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

  void MstState::insertOne(MstState &out_state, const DataType &rhs_batch) {
    auto corresponding = internal_state_.find(rhs_batch);
    if (corresponding == internal_state_.end()) {
      // when state not contains transaction
      rawInsert(rhs_batch);
      return;
    }

    DataType found = *corresponding;
    // Append new signatures to the existing state
    mergeSignaturesInBatch(found, rhs_batch);

    if ((*completer_)(found)) {
      // state already has completed transaction,
      // remove from state and return it
      out_state += found;
      internal_state_.erase(internal_state_.find(found));
    }
  }

  void MstState::rawInsert(const DataType &rhs_batch) {
    internal_state_.insert(rhs_batch);
    index_.push(rhs_batch);
  }

}  // namespace iroha
