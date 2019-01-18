/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_CONSENSUS_INIT_HPP
#define IROHA_CONSENSUS_INIT_HPP

#include <memory>
#include <string>
#include <vector>

#include "ametsuchi/peer_query_factory.hpp"
#include "consensus/consensus_block_cache.hpp"
#include "consensus/yac/outcome_messages.hpp"
#include "consensus/yac/timer.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "consensus/yac/yac.hpp"
#include "consensus/yac/yac_gate.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "consensus/yac/yac_peer_orderer.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "network/block_loader.hpp"
#include "simulator/block_creator.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacInit {
       private:
        // ----------| Yac dependencies |----------

        auto createPeerOrderer(
            std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory);

        auto createNetwork(std::shared_ptr<iroha::network::AsyncGrpcClient<
                               google::protobuf::Empty>> async_call);

        auto createCryptoProvider(
            const shared_model::crypto::Keypair &keypair,
            std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                common_objects_factory);

        auto createTimer(std::chrono::milliseconds delay_milliseconds);

        auto createHashProvider();

        std::shared_ptr<consensus::yac::Yac> createYac(
            ClusterOrdering initial_order,
            const shared_model::crypto::Keypair &keypair,
            std::chrono::milliseconds delay_milliseconds,
            std::shared_ptr<
                iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
                async_call,
            std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                common_objects_factory);

       public:
        std::shared_ptr<YacGate> initConsensusGate(
            std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
            std::shared_ptr<simulator::BlockCreator> block_creator,
            std::shared_ptr<network::BlockLoader> block_loader,
            const shared_model::crypto::Keypair &keypair,
            std::shared_ptr<consensus::ConsensusResultCache> block_cache,
            std::chrono::milliseconds vote_delay_milliseconds,
            std::shared_ptr<
                iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
                async_call,
            std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                common_objects_factory);

        std::shared_ptr<NetworkImpl> consensus_network;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_INIT_HPP
