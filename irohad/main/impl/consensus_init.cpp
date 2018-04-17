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
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      auto YacInit::createPeerOrderer(
          std::shared_ptr<ametsuchi::PeerQuery> wsv) {
        return std::make_shared<PeerOrdererImpl>(wsv);
      }

      auto YacInit::createNetwork() {
        consensus_network = std::make_shared<NetworkImpl>();
        return consensus_network;
      }

      auto YacInit::createCryptoProvider(const shared_model::crypto::Keypair &keypair) {
        auto crypto = std::make_shared<CryptoProviderImpl>(keypair);

        return crypto;
      }

      auto YacInit::createTimer() {
        return std::make_shared<TimerImpl>();
      }

      auto YacInit::createHashProvider() {
        return std::make_shared<YacHashProviderImpl>();
      }

      std::shared_ptr<consensus::yac::Yac> YacInit::createYac(
          ClusterOrdering initial_order,
          const shared_model::crypto::Keypair &keypair,
          std::chrono::milliseconds delay_milliseconds) {
        return Yac::create(YacVoteStorage(),
                           createNetwork(),
                           createCryptoProvider(keypair),
                           createTimer(),
                           initial_order,
                           delay_milliseconds.count());
      }

      std::shared_ptr<YacGate> YacInit::initConsensusGate(
          std::shared_ptr<ametsuchi::PeerQuery> wsv,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader,
          const shared_model::crypto::Keypair &keypair,
          std::chrono::milliseconds vote_delay_milliseconds,
          std::chrono::milliseconds load_delay_milliseconds) {
        auto peer_orderer = createPeerOrderer(wsv);

        auto yac = createYac(peer_orderer->getInitialOrdering().value(),
                             keypair,
                             vote_delay_milliseconds);
        consensus_network->subscribe(yac);

        auto hash_provider = createHashProvider();
        return std::make_shared<YacGateImpl>(std::move(yac),
                                             std::move(peer_orderer),
                                             hash_provider,
                                             block_creator,
                                             block_loader,
                                             load_delay_milliseconds.count());
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
