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
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

using namespace iroha::network;
using namespace iroha::model;

using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::_;

/**
 * @brief Sends data over MstTransportGrpc (MstState and Peer objects) and
 * receives them. When received deserializes them end ensures that deserialized
 * objects equal to objects before sending.
 *
 * @given Initialized transport
 * AND MstState for transfer
 * @when Send state via transport
 * @then Assume that received state same as sent
 */
TEST(TransportTest, SendAndReceive) {
  auto transport = std::make_shared<MstTransportGrpc>();
  auto notifications = std::make_shared<iroha::MockMstTransportNotification>();
  transport->subscribe(notifications);

  std::mutex mtx;
  std::condition_variable cv;
  ON_CALL(*notifications, onNewState(_, _))
      .WillByDefault(
          InvokeWithoutArgs(&cv, &std::condition_variable::notify_one));

  std::shared_ptr<shared_model::interface::Peer> peer =
      makePeer("localhost:50051", "abcdabcdabcdabcdabcdabcdabcdabcd");

  auto state = iroha::MstState::empty();
  state += makeTx(1, iroha::time::now(), makeKey(), 3);
  state += makeTx(1, iroha::time::now(), makeKey(), 4);
  state += makeTx(1, iroha::time::now(), makeKey(), 5);
  state += makeTx(1, iroha::time::now(), makeKey(), 5);

  // we want to ensure that server side will call onNewState()
  // with same parameters as on the client side
  EXPECT_CALL(*notifications, onNewState(_, state))
      .WillOnce(Invoke([&peer](auto &p, auto) { EXPECT_EQ(*p, *peer); }));

  std::unique_ptr<grpc::Server> server;

  grpc::ServerBuilder builder;
  int port = 0;
  builder.AddListeningPort(
      peer->address(), grpc::InsecureServerCredentials(), &port);
  builder.RegisterService(transport.get());
  server = builder.BuildAndStart();
  ASSERT_TRUE(server);
  ASSERT_NE(port, 0);

  transport->sendState(*peer, state);
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait_for(lock, std::chrono::milliseconds(100));

  server->Shutdown();
}
