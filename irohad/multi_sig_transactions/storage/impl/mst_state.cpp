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

#include "multi_sig_transactions/storage/mst_state_time_index.hpp"
#include "multi_sig_transactions/storage/mst_state.hpp"

namespace iroha {
  namespace detail {

    /**
     * Merge collections with unique elements
     * @tparam Collection - type of collection
     * @tparam TargetType - type of elements in collection
     * @tparam Hasher - class for hashing TargetType objects
     * @param left - first collection
     * @param right - second collection
     * @return collection with type Collection, that contain unique union of elements
     */
    template<typename Hasher,
        typename Collection,
        typename TargetType = typename Collection::value_type>
    auto merge_unique(Collection left, Collection right) {
      std::unordered_set<TargetType, Hasher>
          unique_set(left.begin(), left.end());

      unique_set.insert(right.begin(), right.end());
      return Collection(unique_set.begin(), unique_set.end());
    }

    /**
     * Provide merge of sets based on mering same elements
     * @tparam Set - type of set
     * @tparam Merge - type of merge predicate
     * @param left - first set
     * @param right - second set
     * @param merge - merge predicate
     * @return new set, that contains union of elements,
     * where same elements merged inside
     */
    template<typename Set, typename Merge>
    Set set_union(const Set &left, const Set &right, Merge &&merge) {
      Set out;
      out.insert(left.begin(), left.end());
      for (auto &&tx : right) {
        auto iter = out.find(tx);
        if (iter != out.end()) {
          merge(*iter, tx);
        } else {
          out.insert(tx);
        }
      }
      return out;
    }

    /**
     * Provide difference operation on set
     * @tparam Set - type of set
     * @return difference of sets.
     */
    template<typename Set>
    Set set_difference(const Set &left, const Set &right) {
      Set out;
      for (auto &&element : left) {
        if (right.find(element) == right.end()) {
          out.insert(element);
        }
      }
      return out;
    }
  } // namespace detail

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(const CompliterType &completer) {
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
                    detail::set_difference(this->internal_state_,
                                           rhs.internal_state_));
  }

  std::vector<MstState::DataType> MstState::getTransactions() const {
    return std::vector<DataType>(internal_state_.begin(),
                                 internal_state_.end());
  }

  size_t MstState::eraseByTime(const TimeType &time) {
    size_t count;
    return count;
  }

  // ------------------------------| private api |------------------------------

  MstState::MstState(CompliterType completer)
      : MstState(std::move(completer), InternalStateType({})) {
  }

  MstState::MstState(CompliterType completer, InternalStateType transactions)
      : completer_(std::move(completer)),
        internal_state_(std::move(transactions)) {
    log_ = logger::log("MstState");
  }

  void MstState::insertOne(MstState &out_state, const DataType &rhs_tx) {
    auto corresponding = internal_state_.find(rhs_tx);
    if (corresponding == internal_state_.end()) {
      /// when state not contains transaction
      internal_state_.insert(rhs_tx);
      return;
    }

    /// state already contains transaction, merge signatures
    (*corresponding)->signatures =
        detail::merge_unique<iroha::model::SignatureHasher>(
            (*corresponding)->signatures, rhs_tx->signatures);

    if (completer_->operator()(*corresponding)) {
      /// state already has completed transaction,
      /// remove from state and return it
      out_state += *corresponding;
      internal_state_.erase(corresponding);
    }
  }

} // namespace iroha
