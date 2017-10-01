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
   * Provide hashing of signatures based on string representation
   */
  class SignatureHasher {
   public:
    size_t operator()(const iroha::model::Signature &sign) const {
      auto hash = string_hasher(sign.signature.to_string() +
          sign.pubkey.to_string());
      return hash;
    }

   private:
    std::hash<std::string> string_hasher;
  };

  /**
   * Megre collections with unique elements
   * @tparam Collection - type of collection
   * @tparam TargetType - type of elements in collection
   * @tparam Hasher - class for hashing TagetType objects
   * @param left - first collection
   * @param right - second collection
   * @return collection with type Collection, that contain unique union of elements
   */
  template<typename Collection, typename TargetType, typename Hasher>
  auto merge_unique(Collection left, Collection right) {

    std::unordered_set<TargetType, Hasher> unique_set(left.begin(), left.end());

    unique_set.insert(right.begin(), right.end());
    return Collection(unique_set.begin(), unique_set.end());
  }

  /**
   * Provide merge of sets based on mering same elements
   * @tparam Set - type of set
   * @param left - first set
   * @param right - second set
   * @return new set, that contains union of elements,
   * where same elements merged inside
   */
  template<typename Set>
  Set set_union(const Set &left, const Set &right) {
    Set out;
    out.insert(left.begin(), left.end());
    for (auto &&tx : right) {
      auto iter = out.find(tx);
      if (iter != out.end()) {
        // todo move business logic into own template method
        iter->get()->signatures =
            merge_unique<std::vector<iroha::model::Signature>,
                         iroha::model::Signature,
                         SignatureHasher>(iter->get()->signatures,
                                          right.find(tx)->get()->signatures);
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
    // TODO implement diff on set
    return out;
  }
} // anonymous namespace

namespace iroha {

  MstState::MstState() {
    log_ = logger::log("MstState");
  }

  MstState::MstState(InternalStateType transactions) : MstState() {
    internal_state_ = std::move(transactions);
  }

  MstState &MstState::operator+=(const DataType &rhs) {
    auto iter = internal_state_.find(rhs);
    if (iter != internal_state_.end()) {
      iter->get()->signatures =
          merge_unique<std::vector<iroha::model::Signature>,
                       iroha::model::Signature,
                       SignatureHasher>(iter->get()->signatures,
                                        rhs.get()->signatures);
    } else {
      internal_state_.insert(rhs);
    }
    return *this;
  }

  MstState MstState::operator+(const MstState &rhs) const {
    return MstState(set_union(this->internal_state_, rhs.internal_state_));
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(set_difference(this->internal_state_, rhs.internal_state_));
  }

  std::vector<MstState::DataType> MstState::getTransactions() const {
    return std::vector<DataType>(internal_state_.begin(),
                                 internal_state_.end());
  }

} // namespace iroha
