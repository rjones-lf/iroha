/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <grpc++/grpc++.h>
#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_signature_builder.hpp"

using ::testing::_;
using ::testing::An;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;

static size_t num_peers = 1, my_num = 0;

auto mk_local_peer(uint64_t num) {
  auto address = "0.0.0.0:" + std::to_string(num);
  return iroha::consensus::yac::mk_peer(address);
}

class FixedCryptoProvider : public MockYacCryptoProvider {
 public:
  explicit FixedCryptoProvider(const std::string &public_key) {
    std::string key(
        shared_model::crypto::DefaultCryptoAlgorithmType::kPublicKeyLength, 0);
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

  ConsensusSunnyDayTest() : my_peer(mk_local_peer(port + my_num)) {
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

  void SetUp() override {
    auto async_call = std::make_shared<
        iroha::network::AsyncGrpcClient<google::protobuf::Empty>>();
    network = std::make_shared<NetworkImpl>(async_call);
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

  uint64_t delay_before, delay_after;
  std::shared_ptr<shared_model::interface::Peer> my_peer;
  std::vector<std::shared_ptr<shared_model::interface::Peer>> default_peers;
};

/**
 * @given num_peers peers with initialized YAC
 * @when peers vote for same hash
 * @then commit is achieved
 */
TEST_F(ConsensusSunnyDayTest, SunnyDayTest) {
  std::condition_variable cv;
  auto wrapper = make_test_subscriber<CallExact>(yac->onOutcome(), 1);
  wrapper.subscribe([&cv](auto hash) {
    std::cout << "^_^ COMMITTED!!!" << std::endl;
    cv.notify_one();
  });

  EXPECT_CALL(*crypto, verify(_)).WillRepeatedly(Return(true));

  // Wait for other peers to start
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_before));

  YacHash my_hash(iroha::consensus::Round{1, 1}, "proposal_hash", "block_hash");
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
  if (argc == 3) {
    num_peers = std::stoul(argv[1]);
    my_num = std::stoul(argv[2]) + 1;
  }
  return RUN_ALL_TESTS();
}
