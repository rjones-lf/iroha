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

#include "consensus/yac/impl/peer_orderer_impl.hpp"

#include <random>

#include "common/types.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      PeerOrdererImpl::PeerOrdererImpl(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory)
          : peer_query_factory_(peer_query_factory) {}

      boost::optional<ClusterOrdering> PeerOrdererImpl::getInitialOrdering() {
        return peer_query_factory_->createPeerQuery() |
            [](const auto &query) { return query->getLedgerPeers(); } |
            [](const auto &peers) { return ClusterOrdering::create(peers); };
      }

      boost::optional<ClusterOrdering> PeerOrdererImpl::getOrdering(
          const YacHash &hash) {
        return peer_query_factory_->createPeerQuery() |
            [](const auto &query) { return query->getLedgerPeers(); } |
            [&hash](auto peers) {
              std::seed_seq seed(hash.vote_hashes_.block_hash.begin(),
                                 hash.vote_hashes_.block_hash.end());
              std::default_random_engine gen(seed);
              std::shuffle(peers.begin(), peers.end(), gen);
              return ClusterOrdering::create(peers);
            };
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
