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

#include "multi_sig_transactions/storage/mst_storage.hpp"

namespace iroha {
  MstStorage::MstStorage() {
    log_ = logger::log("MstStorage");
  }

  MstState MstStorage::apply(ConstPeer &target_peer, MstState new_state) {
    std::lock_guard<std::mutex> lock{this->mutex_};
    return applyImpl(target_peer, new_state);
  }

  MstState MstStorage::updateOwnState(TransactionType tx) {
    std::lock_guard<std::mutex> lock{this->mutex_};
    return updateOwnStateImpl(std::move(tx));
  }

  MstState MstStorage::getExpiredTransactions(const TimeType &current_time){
    std::lock_guard<std::mutex> lock{this->mutex_};
    return getExpiredTransactionsImpl(current_time);
  }

  MstState MstStorage::getDiffState(ConstPeer &target_peer,
                                    const TimeType &current_time) {
    std::lock_guard<std::mutex> lock(this->mutex_);
    return getDiffStateImpl(target_peer, current_time);
  }
} // namespace iroha
