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
          vote.signature.pubkey = pubkey_;
          return vote;
        }

        MockYacCryptoProvider(model::Peer::KeyType pubkey)
            : pubkey_(std::move(pubkey)) {}

        MockYacCryptoProvider(const MockYacCryptoProvider &): pubkey_{} {}

        MockYacCryptoProvider &operator=(const MockYacCryptoProvider &) {
          return *this;
        }

       private:
        model::Peer::KeyType pubkey_;
      };

      auto YacInit::createNetwork(std::string network_address,
                                  std::vector<model::Peer> initial_peers) {
        consensus_network =
            std::make_shared<NetworkImpl>(network_address, initial_peers);
        return consensus_network;
      }

      auto YacInit::createCryptoProvider(model::Peer::KeyType pubkey) {
        std::shared_ptr<MockYacCryptoProvider> crypto =
            std::make_shared<MockYacCryptoProvider>(pubkey);

        EXPECT_CALL(*crypto, verify(testing::An<CommitMessage>()))
            .WillRepeatedly(testing::Return(true));

        EXPECT_CALL(*crypto, verify(testing::An<RejectMessage>()))
            .WillRepeatedly(testing::Return(true));

        EXPECT_CALL(*crypto, verify(testing::An<VoteMessage>()))
            .WillRepeatedly(testing::Return(true));
        return crypto;
      }

      auto YacInit::createTimer() { return std::make_shared<TimerImpl>(); }

      auto YacInit::createHashProvider() {
        return std::make_shared<YacHashProviderImpl>();
      }

      std::shared_ptr<consensus::yac::Yac> YacInit::createYac(
          std::string network_address,
          ClusterOrdering initial_order) {
        auto &&order = initial_order.getPeers();

        // TODO(@warchant): crypto provider with certificates will be here

        // TODO(@warchant): if find_if does not find pubkey, here will be SEGFAULT
        auto pubkey = std::find_if(order.begin(),
                                   order.end(),
                                   [network_address](auto peer) {
                                     return peer.address == network_address;
                                   })
                          ->pubkey;

        return Yac::create(YacVoteStorage(),
                           createNetwork(std::move(network_address), order),
                           createCryptoProvider(pubkey),
                           createTimer(),
                           initial_order,
                           delay_seconds_ * 1000); // TODO(@warchant): remove magic number

      }

      std::shared_ptr<YacGateImpl> YacInit::initConsensusGate(
          std::string network_address,
          std::shared_ptr<YacPeerOrderer> peer_orderer,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader) {
        auto yac = createYac(std::move(network_address),
                             peer_orderer->getInitialOrdering().value());
        consensus_network->subscribe(yac);

        auto hash_provider = createHashProvider();
        return std::make_shared<YacGateImpl>(
            std::move(yac),
            std::move(peer_orderer),
            hash_provider,
            block_creator,
            block_loader,
            delay_seconds_ * 1000);  // TODO(@warchant): remove magic number
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
