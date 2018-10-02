/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "module/shared_model/interface_mocks.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      std::shared_ptr<shared_model::interface::Peer> mk_peer(
          const std::string &address) {
        auto key = std::string(32, '0');
        std::copy(address.begin(), address.end(), key.begin());
        auto peer = std::make_shared<MockPeer>();
        EXPECT_CALL(*peer, address())
            .WillRepeatedly(::testing::ReturnRefOfCopy(address));
        EXPECT_CALL(*peer, pubkey())
            .WillRepeatedly(::testing::ReturnRefOfCopy(
                shared_model::interface::types::PubkeyType(key)));

        return peer;
      }

      /**
       * Creates test signature with empty signed data, and provided pubkey
       * @param pub_key - public key to put in the signature
       * @return new signature
       */
      std::shared_ptr<shared_model::interface::Signature> createSig(
          const std::string &pub_key) {
        auto tmp =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
                .publicKey();
        std::string key(tmp.blob().size(), 0);
        std::copy(pub_key.begin(), pub_key.end(), key.begin());
        auto sig = std::make_shared<MockSignature>();
        EXPECT_CALL(*sig, publicKey())
            .WillRepeatedly(::testing::ReturnRefOfCopy(
                shared_model::crypto::PublicKey(key)));
        EXPECT_CALL(*sig, signedData())
            .WillRepeatedly(
                ::testing::ReturnRefOfCopy(shared_model::crypto::Signed("")));

        return sig;
      }

      VoteMessage create_vote(YacHash hash, std::string pub_key) {
        VoteMessage vote;
        vote.hash = hash;
        vote.signature = createSig(pub_key);
        return vote;
      }

      class MockYacCryptoProvider : public YacCryptoProvider {
       public:
        MOCK_METHOD1(verify, bool(const std::vector<VoteMessage> &));

        VoteMessage getVote(YacHash hash) override {
          VoteMessage vote;
          vote.hash = hash;
          vote.signature = createSig("");
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
        void invokeAfterDelay(std::function<void()> handler) override {
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

        MOCK_METHOD2(sendState,
                     void(const shared_model::interface::Peer &,
                          const std::vector<VoteMessage> &));

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

        MOCK_METHOD0(onOutcome, rxcpp::observable<Answer>());

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
        MOCK_CONST_METHOD1(makeHash,
                           YacHash(const shared_model::interface::Block &));

        MOCK_CONST_METHOD1(
            toModelHash,
            shared_model::interface::types::HashType(const YacHash &));

        MockYacHashProvider() = default;

        MockYacHashProvider(const MockYacHashProvider &rhs){};

        MockYacHashProvider(MockYacHashProvider &&rhs){};

        MockYacHashProvider &operator=(const MockYacHashProvider &rhs) {
          return *this;
        };
      };

      class MockYacNetworkNotifications : public YacNetworkNotifications {
       public:
        MOCK_METHOD1(onState, void(std::vector<VoteMessage>));
      };

      class MockSupermajorityChecker : public SupermajorityChecker {
       public:
        MOCK_CONST_METHOD2(
            hasSupermajority,
            bool(const shared_model::interface::types::SignatureRangeType
                     &signatures,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &peers));
        MOCK_CONST_METHOD2(checkSize, bool(PeersNumberType, PeersNumberType));
        MOCK_CONST_METHOD2(
            peersSubset,
            bool(const shared_model::interface::types::SignatureRangeType
                     &signatures,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &peers));
        MOCK_CONST_METHOD3(
            hasReject, bool(PeersNumberType, PeersNumberType, PeersNumberType));
      };

      class YacTest : public ::testing::Test {
       public:
        // ------|Network|------
        std::shared_ptr<MockYacNetwork> network;
        std::shared_ptr<MockYacCryptoProvider> crypto;
        std::shared_ptr<MockTimer> timer;
        std::shared_ptr<Yac> yac;

        // ------|Round|------
        std::vector<std::shared_ptr<shared_model::interface::Peer>>
            default_peers = [] {
              std::vector<std::shared_ptr<shared_model::interface::Peer>>
                  result;
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
          initYac(ordering.value());
        }

        void TearDown() override {
          network->release();
        }

        void initYac(ClusterOrdering ordering) {
          yac = Yac::create(YacVoteStorage(), network, crypto, timer, ordering);
          network->subscribe(yac);
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_MOCKS_HPP
