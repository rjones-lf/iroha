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

#include "consensus/yac/cluster_order.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      boost::optional<ClusterOrdering> ClusterOrdering::create(
          const std::vector<model::Peer> &order) {
        if (order.empty()) {
          return boost::none;
        }
        return ClusterOrdering(order);
      }

      ClusterOrdering::ClusterOrdering(std::vector<model::Peer> order)
          : order_(std::move(order)) {}

      model::Peer ClusterOrdering::currentLeader() {
        if (index_ >= order_.size()) {
          index_ = 0;
        }
        return order_.at(index_);
      }

      bool ClusterOrdering::hasNext() const {
        return index_ != order_.size();
      }

      ClusterOrdering &ClusterOrdering::switchToNext() {
        ++index_;
        return *this;
      }

      std::vector<model::Peer> ClusterOrdering::getPeers() const {
        return order_;
      }

      size_t ClusterOrdering::getNumberOfPeers() const {
        return order_.size();
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
