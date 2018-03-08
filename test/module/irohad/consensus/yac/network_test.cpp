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

#include "module/irohad/consensus/yac/yac_mocks.hpp"

#include <grpc++/grpc++.h>

#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "consensus/yac/transport/yac_pb_converters.hpp"

using ::testing::_;
using ::testing::InvokeWithoutArgs;

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacNetworkTest : public ::testing::Test {
       public:
        static constexpr auto default_ip = "0.0.0.0";
        static constexpr auto default_address = "0.0.0.0:0";
        void SetUp() override {
          notifications = std::make_shared<MockYacNetworkNotifications>();

          network = std::make_shared<NetworkImpl>();

          message.hash.proposal_hash = "proposal";
          message.hash.block_hash = "block";

          network->subscribe(notifications);

          grpc::ServerBuilder builder;
          int port = 0;
          builder.AddListeningPort(
              default_address, grpc::InsecureServerCredentials(), &port);
          builder.RegisterService(network.get());
          server = builder.BuildAndStart();
          ASSERT_TRUE(server);
          ASSERT_NE(port, 0);

          peer = mk_peer(std::string(default_ip) + ":" + std::to_string(port));
        }

        void TearDown() override {
          server->Shutdown();
        }

        std::shared_ptr<MockYacNetworkNotifications> notifications;
        std::shared_ptr<NetworkImpl> network;
        std::shared_ptr<shared_model::interface::Peer> peer;
        VoteMessage message;
        std::unique_ptr<grpc::Server> server;
        std::mutex mtx;
        std::condition_variable cv;
      };

      /**
       * @given initialized network
       * @when send vote to itself
       * @then vote handled
       */
      TEST_F(YacNetworkTest, MessageHandledWhenMessageSent) {
        EXPECT_CALL(*notifications, on_vote(message))
            .Times(1)
            .WillRepeatedly(
                InvokeWithoutArgs(&cv, &std::condition_variable::notify_one));

        network->send_vote(*peer, message);

        // wait for response reader thread
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(100));
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
