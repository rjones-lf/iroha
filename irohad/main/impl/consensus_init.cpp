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

#include "main/impl/consensus_init.hpp"

#include "consensus/yac/impl/peer_orderer_impl.hpp"
#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      auto YacInit::createPeerOrderer(
          std::shared_ptr<ametsuchi::PeerQuery> wsv) {
        return std::make_shared<PeerOrdererImpl>(wsv);
      }

      auto YacInit::createNetwork(std::string network_address,
                                  std::vector<model::Peer> initial_peers) {
        consensus_network =
            std::make_shared<NetworkImpl>(network_address, initial_peers);
        return consensus_network;
      }

      auto YacInit::createCryptoProvider(const keypair_t &keypair) {
        auto crypto = std::make_shared<CryptoProviderImpl>(keypair);

        return crypto;
      }

      auto YacInit::createTimer(std::shared_ptr<uvw::Loop> loop) {
        return std::make_shared<TimerImpl>(loop);
      }

      auto YacInit::createHashProvider() {
        return std::make_shared<YacHashProviderImpl>();
      }

      std::shared_ptr<consensus::yac::Yac> YacInit::createYac(
          std::string network_address,
          std::shared_ptr<uvw::Loop> loop,
          ClusterOrdering initial_order,
          const keypair_t &keypair) {
        return Yac::create(
            YacVoteStorage(),
            createNetwork(std::move(network_address), initial_order.getPeers()),
            createCryptoProvider(keypair),
            createTimer(std::move(loop)),
            initial_order,
            delay_seconds_ * 1000);
      }

      std::shared_ptr<YacGate> YacInit::initConsensusGate(
          std::string network_address,
          std::shared_ptr<uvw::Loop> loop,
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader,
          const keypair_t &keypair) {
        auto peer_orderer = createPeerOrderer(wsv);

        auto yac = createYac(std::move(network_address),
                             std::move(loop),
                             peer_orderer->getInitialOrdering().value(),
                             keypair);
        consensus_network->subscribe(yac);

        auto hash_provider = createHashProvider();
        return std::make_shared<YacGateImpl>(std::move(yac),
                                             std::move(peer_orderer),
                                             hash_provider,
                                             block_creator,
                                             block_loader,
                                             delay_seconds_ * 1000);
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
