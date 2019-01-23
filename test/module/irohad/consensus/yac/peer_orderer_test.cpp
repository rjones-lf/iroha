/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/peer_orderer_impl.hpp"

#include <iostream>
#include <unordered_map>

#include <boost/accumulators/accumulators.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/numeric.hpp>
#include "consensus/yac/storage/yac_proposal_storage.hpp"

#include "module/irohad/ametsuchi/mock_peer_query.hpp"
#include "module/irohad/ametsuchi/mock_peer_query_factory.hpp"
#include "module/irohad/consensus/yac/yac_test_util.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace boost::adaptors;
using namespace iroha::ametsuchi;
using namespace iroha::consensus::yac;

using namespace std;
using ::testing::Return;
using ::testing::ReturnRefOfCopy;

using wPeer = std::shared_ptr<shared_model::interface::Peer>;

size_t N_PEERS = 4;

class YacPeerOrdererTest : public ::testing::Test {
 public:
  YacPeerOrdererTest() : orderer(make_shared<MockPeerQueryFactory>()) {}

  void SetUp() override {
    wsv = make_shared<MockPeerQuery>();
    pbfactory = make_shared<MockPeerQueryFactory>();
    EXPECT_CALL(*pbfactory, createPeerQuery())
        .WillRepeatedly(testing::Return(boost::make_optional(
            std::shared_ptr<iroha::ametsuchi::PeerQuery>(wsv))));
    orderer = PeerOrdererImpl(pbfactory);
  }

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers = [] {
    std::vector<std::shared_ptr<shared_model::interface::Peer>> result;
    for (size_t i = 1; i <= N_PEERS; ++i) {
      auto peer = std::make_shared<MockPeer>();
      EXPECT_CALL(*peer, address())
          .WillRepeatedly(ReturnRefOfCopy(std::to_string(i)));
      EXPECT_CALL(*peer, pubkey())
          .WillRepeatedly(
              ReturnRefOfCopy(shared_model::interface::types::PubkeyType(
                  std::string(32, '0'))));

      result.push_back(peer);
    }
    return result;
  }();

  std::vector<wPeer> s_peers = [] {
    std::vector<wPeer> result;
    for (size_t i = 1; i <= N_PEERS; ++i) {
      auto tmp = iroha::consensus::yac::makePeer(std::to_string(i));

      auto key = tmp->pubkey();

      auto peer = std::make_shared<MockPeer>();
      EXPECT_CALL(*peer, address())
          .WillRepeatedly(ReturnRefOfCopy(tmp->address()));
      EXPECT_CALL(*peer, pubkey()).WillRepeatedly(ReturnRefOfCopy(key));

      result.emplace_back(peer);
    }
    return result;
  }();

  shared_ptr<MockPeerQuery> wsv;
  shared_ptr<MockPeerQueryFactory> pbfactory;
  PeerOrdererImpl orderer;
};

TEST_F(YacPeerOrdererTest, PeerOrdererInitialOrderWhenInvokeNormalCase) {
  cout << "----------| InitialOrder() => valid object |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(s_peers));
  auto order = orderer.getInitialOrdering();

  ASSERT_EQ(order.value().getPeers(), s_peers);
}

TEST_F(YacPeerOrdererTest, PeerOrdererInitialOrderWhenInvokeFailCase) {
  cout << "----------| InitialOrder() => nullopt case |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(boost::none));
  auto order = orderer.getInitialOrdering();
  ASSERT_EQ(order, boost::none);
}

TEST_F(YacPeerOrdererTest, PeerOrdererOrderingWhenInvokeNormalCase) {
  cout << "----------| Order() => valid object |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(s_peers));
  auto order = orderer.getOrdering(YacHash());
  ASSERT_EQ(order.value().getPeers().size(), peers.size());
}

TEST_F(YacPeerOrdererTest, PeerOrdererOrderingWhenInvokeFaillCase) {
  cout << "----------| Order() => nullopt case |----------" << endl;

  EXPECT_CALL(*wsv, getLedgerPeers()).WillOnce(Return(boost::none));
  auto order = orderer.getOrdering(YacHash());
  ASSERT_EQ(order, boost::none);
}

/**
 * @given initial peer list in the ledger
 * @when calling ordering function on hash ["1" to "N"] k times
 * @test histogram must be from uniform distibution
 */
TEST_F(YacPeerOrdererTest, FairnessTest) {
  // Calculate number of permutations of peers
  double comb = std::tgamma(N_PEERS + 1);
  // Run experiments N times for each combination
  double exp_val = 30;
  int times = comb * exp_val;
  std::unordered_map<std::string, int> histogram;
  EXPECT_CALL(*wsv, getLedgerPeers())
      .Times(times)
      .WillRepeatedly(Return(s_peers));

  auto peers_set =
      transform(boost::counting_range(1, times + 1), [this](const auto &i) {
        std::string hash = std::to_string(i);
        return orderer
            .getOrdering(YacHash(iroha::consensus::Round{1, 1}, hash, hash))
            .value()
            .getPeers();
      });
  for (const auto &peers : peers_set) {
    std::string res = std::accumulate(peers.begin(),
                                      peers.end(),
                                      std::string(),
                                      [](std::string res, const auto &peer) {
                                        return res + " " + peer->address();
                                      });
    histogram[res]++;
  }
  // chi square goodness of fit test
  auto chi_square = boost::accumulate(
      histogram | map_values, 0.0, [&exp_val](auto stat, const auto &val) {
        return stat + (val - exp_val) * (val - exp_val) / exp_val;
      });
  boost::math::chi_squared chi_dist(comb - 1);
  auto p = boost::math::cdf(chi_dist, chi_square);
  // Significance level 0.05 is used
  ASSERT_LT(0.05, p) << "The distibution is not uniform, p = "
                     << std::to_string(p);
  ASSERT_EQ(comb, histogram.size());
}
