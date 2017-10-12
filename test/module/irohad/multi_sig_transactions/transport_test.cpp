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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "mst_test_helpers.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

using namespace iroha::network;
using namespace iroha::model;

using ::testing::AtLeast;
using ::testing::_;
using ::testing::InvokeWithoutArgs;

class MockMstTransportNotification : public MstTransportNotification {
 public:
  MOCK_METHOD2(onStateUpdate, void(Peer peer, iroha::MstState state));
};

TEST(TransportTest, SendAndReceive) {
  auto transport = std::make_shared<MstTransportGrpc>();
  auto notifications = std::make_shared<MockMstTransportNotification>();
  transport->subscribe(notifications);

  std::mutex mtx;
  std::condition_variable cv;
  ON_CALL(*notifications, onStateUpdate(_, _))
      .WillByDefault(
          InvokeWithoutArgs(&cv, &std::condition_variable::notify_one));

  auto peer = makePeer("localhost:50051", "abcdabcdabcdabcdabcdabcdabcdabcd");

  auto tx1 = makeTx("1", "4", 3);
  auto tx2 = makeTx("2", "5", 4);
  auto tx3 = makeTx("3", "6", 5);
  auto tx4 = Transaction{};
  tx4.creator_account_id = "me";
  tx4.quorum = 11;
  tx4.created_ts = 12345;

  MstState state = MstState::empty();
  state += tx1;
  state += tx2;
  state += tx3;
  state += std::make_shared<Transaction>(tx4);

  EXPECT_CALL(*notifications, onStateUpdate(peer, state)).Times(1);

  std::unique_ptr<grpc::Server> server;

  grpc::ServerBuilder builder;
  int port = 0;
  builder.AddListeningPort(peer.address, grpc::InsecureServerCredentials(),
                           &port);
  builder.RegisterService(transport.get());
  server = builder.BuildAndStart();
  ASSERT_TRUE(server);
  ASSERT_NE(port, 0);

  transport->sendState(peer, state);
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait_for(lock, std::chrono::milliseconds(100));

  server->Shutdown();
}
