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

#include <grpc++/grpc++.h>

#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_signature_builder.hpp"

using ::testing::An;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;

auto mk_local_peer(uint64_t num) {
  auto address = "0.0.0.0:" + std::to_string(num);
  return iroha::consensus::yac::mk_peer(address);
}

class FixedCryptoProvider : public MockYacCryptoProvider {
 public:
  explicit FixedCryptoProvider(const std::string &public_key) {
    // TODO 15.04.2018 x3medima17 IR-1189: move to separate class
    auto size =
        shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
            .publicKey()
            .size();
    // TODO 16.04.2018 x3medima17 IR-977: add sizes
    std::string key(size, 0);
    std::copy(public_key.begin(), public_key.end(), key.begin());
    pubkey = clone(shared_model::crypto::PublicKey(key));
  }

  VoteMessage getVote(YacHash hash) override {
    auto vote = MockYacCryptoProvider::getVote(hash);
    vote.signature = clone(TestSignatureBuilder().publicKey(*pubkey).build());
    return vote;
  }

  std::unique_ptr<shared_model::crypto::PublicKey> pubkey;
};

class ConsensusSunnyDayTest : public ::testing::Test {
 public:
  std::unique_ptr<grpc::Server> server;
  std::shared_ptr<NetworkImpl> network;
  std::shared_ptr<MockYacCryptoProvider> crypto;
  std::shared_ptr<TimerImpl> timer;
  uint64_t delay = 3 * 1000;
  std::shared_ptr<Yac> yac;

  static const size_t port = 50541;

  void SetUp() override {
    network = std::make_shared<NetworkImpl>();
    crypto = std::make_shared<FixedCryptoProvider>(std::to_string(my_num));
    timer = std::make_shared<TimerImpl>([this] {
      // static factory with a single thread
      // see YacInit::createTimer in consensus_init.cpp
      static rxcpp::observe_on_one_worker coordination(
          rxcpp::observe_on_new_thread().create_coordinator().get_scheduler());
      return rxcpp::observable<>::timer(std::chrono::milliseconds(delay),
                                        coordination);
    });
    auto order = ClusterOrdering::create(default_peers);
    ASSERT_TRUE(order);

    yac = Yac::create(YacVoteStorage(), network, crypto, timer, order.value());
    network->subscribe(yac);

    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort(
        my_peer->address(), grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(network.get());
    server = builder.BuildAndStart();
    ASSERT_TRUE(server);
    ASSERT_NE(port, 0);
  }

  void TearDown() override {
    server->Shutdown();
  }

  static uint64_t my_num, delay_before, delay_after;
  static std::shared_ptr<shared_model::interface::Peer> my_peer;
  static std::vector<std::shared_ptr<shared_model::interface::Peer>>
      default_peers;

  static void init(uint64_t num_peers, uint64_t num) {
    my_num = num;
    my_peer = mk_local_peer(port + my_num);
    for (decltype(num_peers) i = 0; i < num_peers; ++i) {
      default_peers.push_back(mk_local_peer(port + i));
    }
    if (num_peers == 1) {
      delay_before = 0;
      delay_after = 5 * 1000;
    } else {
      delay_before = 10 * 1000;
      delay_after = 3 * default_peers.size() + 10 * 1000;
    }
  }
};

uint64_t ConsensusSunnyDayTest::my_num;
uint64_t ConsensusSunnyDayTest::delay_before;
uint64_t ConsensusSunnyDayTest::delay_after;
std::shared_ptr<shared_model::interface::Peer> ConsensusSunnyDayTest::my_peer;
std::vector<std::shared_ptr<shared_model::interface::Peer>>
    ConsensusSunnyDayTest::default_peers;

TEST_F(ConsensusSunnyDayTest, SunnyDayTest) {
  std::condition_variable cv;
  auto wrapper = make_test_subscriber<CallExact>(yac->on_commit(), 1);
  wrapper.subscribe(
      [](auto hash) { std::cout << "^_^ COMMITTED!!!" << std::endl; });

  EXPECT_CALL(*crypto, verify(An<CommitMessage>()))
      .Times(1)
      .WillRepeatedly(DoAll(InvokeWithoutArgs([&cv] {
                              // wake up after commit is received from the
                              // network so that it is safe to shutdown
                              cv.notify_one();
                            }),
                            Return(true)));
  EXPECT_CALL(*crypto, verify(An<VoteMessage>())).WillRepeatedly(Return(true));

  // Wait for other peers to start
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_before));

  YacHash my_hash("proposal_hash", "block_hash");
  my_hash.block_signature = createSig("");
  auto order = ClusterOrdering::create(default_peers);
  ASSERT_TRUE(order);

  yac->vote(my_hash, *order);
  std::mutex m;
  std::unique_lock<std::mutex> lk(m);
  cv.wait_for(lk, std::chrono::milliseconds(delay_after));

  ASSERT_TRUE(wrapper.validate());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  uint64_t num_peers = 1, my_num = 0;
  if (argc == 3) {
    num_peers = std::stoul(argv[1]);
    my_num = std::stoul(argv[2]) + 1;
  }
  ConsensusSunnyDayTest::init(num_peers, my_num);
  return RUN_ALL_TESTS();
}
