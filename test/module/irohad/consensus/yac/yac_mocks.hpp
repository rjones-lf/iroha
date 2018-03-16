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

#ifndef IROHA_YAC_MOCKS_HPP
#define IROHA_YAC_MOCKS_HPP

#include <gmock/gmock.h>

#include "common/byteutils.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/supermajority_checker.hpp"
#include "consensus/yac/timer.hpp"
#include "consensus/yac/yac.hpp"
#include "consensus/yac/yac_crypto_provider.hpp"
#include "consensus/yac/yac_gate.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "consensus/yac/yac_peer_orderer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      model::Peer mk_peer(std::string address) {
        model::Peer peer;
        peer.address = address;
        // TODO: 19.01.2019 kamil substitute with function, IR-813
        std::copy(address.begin(), address.end(), peer.pubkey.begin());
        return peer;
      }

      VoteMessage create_vote(YacHash hash, std::string pub_key) {
        VoteMessage vote;
        vote.hash = hash;
        // TODO: 19.01.2019 kamil substitute with function, IR-813
        std::copy(
            pub_key.begin(), pub_key.end(), vote.signature.pubkey.begin());
        return vote;
      }

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

      class MockTimer : public Timer {
       public:
        void invokeAfterDelay(uint64_t millis,
                              std::function<void()> handler) override {
          handler();
        }

        MOCK_METHOD0(deny, void());

        MockTimer() = default;

        MockTimer(const MockTimer &rhs) {}

        MockTimer &operator=(const MockTimer &rhs) {
          return *this;
        }
      };

      class MockYacNetwork : public YacNetwork {
       public:
        void subscribe(
            std::shared_ptr<YacNetworkNotifications> handler) override {
          notification = handler;
        };

        void release() {
          notification.reset();
        }

        MOCK_METHOD2(send_commit, void(model::Peer, CommitMessage));
        MOCK_METHOD2(send_reject, void(model::Peer, RejectMessage));
        MOCK_METHOD2(send_vote, void(model::Peer, VoteMessage));

        MockYacNetwork() = default;

        MockYacNetwork(const MockYacNetwork &rhs)
            : notification(rhs.notification) {}

        MockYacNetwork &operator=(const MockYacNetwork &rhs) {
          notification = rhs.notification;
          return *this;
        }

        MockYacNetwork(MockYacNetwork &&rhs) {
          std::swap(notification, rhs.notification);
        }

        MockYacNetwork &operator=(MockYacNetwork &&rhs) {
          std::swap(notification, rhs.notification);
          return *this;
        }

        std::shared_ptr<YacNetworkNotifications> notification;
      };

      class MockHashGate : public HashGate {
       public:
        MOCK_METHOD2(vote, void(YacHash, ClusterOrdering));

        MOCK_METHOD0(on_commit, rxcpp::observable<CommitMessage>());

        MockHashGate() = default;

        MockHashGate(const MockHashGate &rhs) {}

        MockHashGate(MockHashGate &&rhs) {}

        MockHashGate &operator=(const MockHashGate &rhs) {
          return *this;
        };
      };

      class MockYacPeerOrderer : public YacPeerOrderer {
       public:
        MOCK_METHOD0(getInitialOrdering, boost::optional<ClusterOrdering>());

        MOCK_METHOD1(getOrdering,
                     boost::optional<ClusterOrdering>(const YacHash &));

        MockYacPeerOrderer() = default;

        MockYacPeerOrderer(const MockYacPeerOrderer &rhs){};

        MockYacPeerOrderer(MockYacPeerOrderer &&rhs){};

        MockYacPeerOrderer &operator=(const MockYacPeerOrderer &rhs) {
          return *this;
        };
      };

      class MockYacHashProvider : public YacHashProvider {
       public:
        MOCK_CONST_METHOD1(makeHash, YacHash(const model::Block &));

        MOCK_CONST_METHOD1(toModelHash,
                           model::Block::HashType(const YacHash &));

        MockYacHashProvider() = default;

        MockYacHashProvider(const MockYacHashProvider &rhs){};

        MockYacHashProvider(MockYacHashProvider &&rhs){};

        MockYacHashProvider &operator=(const MockYacHashProvider &rhs) {
          return *this;
        };
      };

      class MockYacNetworkNotifications : public YacNetworkNotifications {
       public:
        MOCK_METHOD1(on_commit, void(CommitMessage));
        MOCK_METHOD1(on_reject, void(RejectMessage));
        MOCK_METHOD1(on_vote, void(VoteMessage));
      };

      class MockSupermajorityChecker : public SupermajorityChecker {
       public:
        MOCK_CONST_METHOD2(
            hasSupermajority,
            bool(const shared_model::interface::SignatureSetType &signatures,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &peers));
        MOCK_CONST_METHOD2(checkSize, bool(uint64_t current, uint64_t all));
        MOCK_CONST_METHOD2(
            peersSubset,
            bool(const shared_model::interface::SignatureSetType &signatures,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &peers));
        MOCK_CONST_METHOD3(
            hasReject, bool(uint64_t frequent, uint64_t voted, uint64_t all));
      };

      class YacTest : public ::testing::Test {
       public:
        // ------|Network|------
        std::shared_ptr<MockYacNetwork> network;
        std::shared_ptr<MockYacCryptoProvider> crypto;
        std::shared_ptr<MockTimer> timer;
        uint64_t delay = 100500;
        std::shared_ptr<Yac> yac;

        // ------|Round|------
        std::vector<model::Peer> default_peers = [] {
          std::vector<model::Peer> result;
          for (size_t i = 1; i <= 7; ++i) {
            result.push_back(mk_peer(std::to_string(i)));
          }
          return result;
        }();

        void SetUp() override {
          network = std::make_shared<MockYacNetwork>();
          crypto = std::make_shared<MockYacCryptoProvider>();
          timer = std::make_shared<MockTimer>();
          auto ordering = ClusterOrdering::create(default_peers);
          ASSERT_TRUE(ordering);
          yac = Yac::create(YacVoteStorage(),
                            network,
                            crypto,
                            timer,
                            ordering.value(),
                            delay);
          network->subscribe(yac);
        };

        void TearDown() override {
          network->release();
        };
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_MOCKS_HPP
