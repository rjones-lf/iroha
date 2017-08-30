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

#ifndef IROHA_CONSENSUS_INIT_HPP
#define IROHA_CONSENSUS_INIT_HPP

#include <memory>
#include <string>
#include <vector>
#include "consensus/yac/yac.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/impl/network_impl.hpp"
#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/impl/peer_orderer_impl.hpp"
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacInit {
       private:
        // ----------| Yac dependencies |----------

        auto createNetwork(std::string network_address,
                           std::vector<model::Peer> initial_peers);

        auto createCryptoProvider(model::Peer::KeyType pubkey);

        auto createTimer(std::shared_ptr<uvw::Loop> loop);

        auto createHashProvider();

        std::shared_ptr<consensus::yac::Yac> createYac(std::string network_address,
                                                       std::shared_ptr<uvw::Loop> loop,
                                                       ClusterOrdering initial_order);

       public:
        std::shared_ptr<YacGateImpl> initConsensusGate(std::string network_address,
                               std::shared_ptr<uvw::Loop> loop,
                               std::shared_ptr<YacPeerOrderer> peer_orderer,
                               std::shared_ptr<simulator::BlockCreator> block_creator,
                               std::shared_ptr<network::BlockLoader> block_loader);

        std::shared_ptr<NetworkImpl> consensus_network;
      };
    } // namespace yac
  } // namespace consensus
} // namespace iroha

#endif //IROHA_CONSENSUS_INIT_HPP
