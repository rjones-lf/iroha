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

#include "multi_sig_transactions/storage/mst_state.hpp"

namespace {

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
    std::unordered_set<TargetType, Hasher> unique_set(left.begin(), left.end());

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
} // anonymous namespace

namespace iroha {

  MstState::MstState() : MstState(InternalStateType{}) {
  }

  MstState &MstState::operator+=(const DataType &rhs) {
    auto iter = internal_state_.find(rhs);
    if (iter != internal_state_.end()) {
      (*iter)->signatures =
          merge_unique<iroha::model::SignatureHasher>((*iter)->signatures,
                                                      rhs->signatures);
    } else {
      internal_state_.insert(rhs);
    }
    return *this;
  }

  MstState MstState::operator+(const MstState &rhs) const {
    return MstState(set_union(
        this->internal_state_, rhs.internal_state_, [](auto iter, auto tx) {
          iter->signatures =
              merge_unique<iroha::model::SignatureHasher>(iter->signatures,
                                                          tx->signatures);
        }));
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(set_difference(this->internal_state_, rhs.internal_state_));
  }

  std::vector<MstState::DataType> MstState::getTransactions() const {
    return std::vector<DataType>(internal_state_.begin(),
                                 internal_state_.end());
  }

  MstState::MstState(InternalStateType transactions)
      : internal_state_(std::move(transactions)) {
    log_ = logger::log("MstState");
  }

} // namespace iroha
