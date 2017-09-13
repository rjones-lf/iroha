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

#include <grpc++/grpc++.h>
#include <gtest/gtest.h>
#include <atomic>
#include <crypto/crypto.hpp>
#include <peer_service/service.hpp>
#include <thread>
#define NPEERS 2

using namespace iroha;

using SERVICE = std::pair<std::unique_ptr<grpc::ServerBuilder>,
                          std::unique_ptr<peerservice::PeerServiceImpl>>;

class Service : public testing::Test {
 public:
  Service() {
    std::vector<ed25519::keypair_t> kps(NPEERS);
    std::generate_n(kps.begin(), NPEERS, create_keypair);

    std::vector<peerservice::Node> cluster;
    for (int i = 0; i < NPEERS; ++i) {
      peerservice::Node node;
      node.ip = "127.0.0.1";
      node.port = (uint16_t)(65530 + i);
      node.pubkey = kps[i].pubkey;
      cluster.push_back(node);
    }

    for (int i = 0; i < NPEERS; ++i) {
      auto &&builder = std::make_unique<grpc::ServerBuilder>();
      auto on = cluster[i].ip + ":" + std::to_string(cluster[i].port);

      printf("LISTEN ON: %s\n", on.c_str());

      builder->AddListeningPort(on, grpc::InsecureServerCredentials());

      auto &&loop = uvw::Loop::create();
      auto &&service = std::make_unique<peerservice::PeerServiceImpl>(
          cluster, cluster[i].pubkey, std::move(loop));

      builder->RegisterService(service.get());

      auto &&pair = std::make_pair(std::move(builder), std::move(service));

      this->services.push_back(std::move(pair));
    }
  }

  std::vector<SERVICE> services;
};

/*
TEST_F(Service, AyoungerThanB) {
  auto &&A = services[0].second;
  auto &&B = services[1].second;

  using peerservice::Heartbeat;

  size_t HEIGHT = 3;

  // heartbeat for A
  Heartbeat a;
  auto aPub = A->getMyNode().pubkey.to_string();
  a.set_height(HEIGHT - 1);  // smaller than HEIGHT, A's ledger has less blocks
  a.set_gmroot(
      std::string(iroha::hash256_t::size(), 'a'));  // 32 bytes: aaaaaaaa...aa
  a.set_allocated_pubkey(&aPub);

  // heartbeat for B
  Heartbeat b;
  auto bPub = B->getMyNode().pubkey.to_string();
  b.set_gmroot(
      std::string(iroha::hash256_t::size(), 'b'));  // 32 bytes: bbbbbbbb...bb
  b.set_height(HEIGHT);
  b.set_allocated_pubkey(&bPub);

  A->setMyState(a);
  B->setMyState(b);

  A->on<Heartbeat>([HEIGHT](const Heartbeat &hb, auto &t) {
    ASSERT_EQ(HEIGHT, hb.height())
        << "we recv higher ledger, but heights are different";
  });

  A->ping();

  //    std::this_thread::sleep_for(std::chrono::seconds(5));

  auto online = A->getOnlineNodes();
  // NPEERS - 1 because we do not ping ourself
  EXPECT_EQ(online.size(), NPEERS - 1) << "not all peers online";

  EXPECT_TRUE(A->getLoop()->alive()) << "A's loop is dead";
  EXPECT_TRUE(B->getLoop()->alive()) << "B's loop is dead";

  ASSERT_EQ(B->getLatestState().height(), HEIGHT);
  ASSERT_EQ(A->getLatestState().height(), HEIGHT)
      << "A(2) -> B(3) and  A did not update its latest state";
}
 */
