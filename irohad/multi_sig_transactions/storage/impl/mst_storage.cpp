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

#include "multi_sig_transactions/storage/mst_storage.hpp"

namespace iroha {
  MstStorage::MstStorage() {
    log_ = logger::log("MstStorage");
  }

  void MstStorage::apply(const model::Peer &target_peer, MstState new_state) {
    std::lock_guard<std::mutex> _{this->mutex_};
    return applyImpl(target_peer, std::move(new_state));
  }

  void MstStorage::updateOwnState(TransactionType tx) {
    std::lock_guard<std::mutex> _{this->mutex_};
    return updateOwnStateImpl(std::move(tx));
  }

  MstState MstStorage::getDiffState(const model::Peer &target_peer) const {
    std::lock_guard<std::mutex> _{this->mutex_};
    return getDiffStateImpl(target_peer);
  }
} // namespace iroha
