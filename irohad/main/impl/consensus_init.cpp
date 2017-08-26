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

#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "consensus/yac/yac.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/impl/network_impl.hpp"
#include <consensus/yac/impl/timer_impl.hpp>
#include <consensus/yac/impl/peer_orderer_impl.hpp>
#include <consensus/yac/impl/yac_hash_provider_impl.hpp>

namespace iroha {
  namespace consensus {
    namespace yac {

      class MockYacCryptoProvider : public YacCryptoProvider {
       public:
        MOCK_METHOD1(verify, bool(CommitMessage));
        MOCK_METHOD1(verify, bool(RejectMessage));
        MOCK_METHOD1(verify, bool(VoteMessage));

        VoteMessage getVote(YacHash hash) override {
          VoteMessage vote;
          vote.hash = hash;
          return vote;
        }

        MockYacCryptoProvider() = default;

        MockYacCryptoProvider(const MockYacCryptoProvider &) {}

        MockYacCryptoProvider &operator=(const MockYacCryptoProvider &) {
          return *this;
        }
      };

      auto YacInit::createNetwork(std::string network_address,
                                  std::vector<model::Peer> initial_peers) {
        consensus_network = std::make_shared<NetworkImpl>(network_address, initial_peers);
        return consensus_network;
      }

      auto YacInit::createCryptoProvider() {
        std::shared_ptr<MockYacCryptoProvider>
            crypto = std::make_shared<MockYacCryptoProvider>();

        EXPECT_CALL(*crypto, verify(testing::An<CommitMessage>()))
            .WillRepeatedly(testing::Return(true));

        EXPECT_CALL(*crypto, verify(testing::An<RejectMessage>()))
            .WillRepeatedly(testing::Return(true));

        EXPECT_CALL(*crypto, verify(testing::An<VoteMessage>()))
            .WillRepeatedly(testing::Return(true));
        return crypto;
      }

      auto YacInit::createTimer(std::shared_ptr<uvw::Loop> loop) {
        return std::make_shared<TimerImpl>(loop);
      }

      auto YacInit::createHashProvider() {
        return std::make_shared<YacHashProviderImpl>();
      }

      std::shared_ptr<consensus::yac::Yac> YacInit::createYac(std::string network_address,
                                                              std::shared_ptr<
                                                                  uvw::Loop> loop,
                                                              ClusterOrdering initial_order) {
        uint64_t delay_seconds = 5;

        return Yac::create(YacVoteStorage(),
                           createNetwork(std::move(network_address),
                                         initial_order.getPeers()),
                           createCryptoProvider(),
                           createTimer(std::move(loop)),
                           initial_order,
                           delay_seconds * 1000);

      }

      std::shared_ptr<YacGateImpl> YacInit::initConsensusGate(std::string network_address,
                                  std::shared_ptr<uvw::Loop> loop,
                                  std::shared_ptr<YacPeerOrderer> peer_orderer,
                                  std::shared_ptr<simulator::BlockCreator> block_creator,
                                  std::shared_ptr<network::BlockLoader> block_loader) {
        auto yac = createYac(std::move(network_address),
                             std::move(loop),
                             peer_orderer->getInitialOrdering().value());
        consensus_network->subscribe(yac);

        auto hash_provider = createHashProvider();
        return std::make_shared<YacGateImpl>(std::move(yac),
                                             std::move(peer_orderer),
                                             hash_provider,
                                             block_creator,
                                             block_loader);
      }

    } // namespace yac
  } // namespace consensus
} // namespace iroha
