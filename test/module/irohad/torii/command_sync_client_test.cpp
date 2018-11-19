/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"
#include "main/server_runner.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "torii/command_client.hpp"

using testing::_;
using testing::Invoke;
using testing::Return;

class SyncClient : public testing::Test {
 public:
  void SetUp() override {
    runner = std::make_unique<ServerRunner>(ip + ":0");
    server = std::make_shared<iroha::torii::MockCommandServiceTransport>();
    runner->append(server).run().match(
        [this](iroha::expected::Value<int> port) { this->port = port.value; },
        [](iroha::expected::Error<std::string> err) { FAIL() << err.error; });
    runner->waitForServersReady();
  }

  std::unique_ptr<ServerRunner> runner;
  std::shared_ptr<iroha::torii::MockCommandServiceTransport> server;

  const std::string ip = "127.0.0.1";
  const size_t kHashLength =
      shared_model::crypto::CryptoProviderEd25519Sha3::kHashLength;
  int port;
};

/**
 * @given command client
 * @when Status is called
 * @then the same method of the server is called and client successfully return
 */
TEST_F(SyncClient, Status) {
  iroha::protocol::TxStatusRequest tx_request;
  tx_request.set_tx_hash(std::string(kHashLength, '1'));
  iroha::protocol::ToriiResponse toriiResponse;

  torii::CommandSyncClient client(ip, port);
  EXPECT_CALL(*server, Status(_, _, _)).WillOnce(Return(grpc::Status::OK));
  auto stat = client.Status(tx_request, toriiResponse);
  ASSERT_TRUE(stat.ok());
}

/**
 * @given command client
 * @when Torii is called
 * @then the same method of the server is called and client successfully return
 */
TEST_F(SyncClient, Torii) {
  iroha::protocol::Transaction tx;
  EXPECT_CALL(*server, Torii(_, _, _)).WillOnce(Return(grpc::Status()));
  torii::CommandSyncClient client(ip, port);
  auto stat = client.Torii(tx);
  ASSERT_TRUE(stat.ok());
}

/**
 * @given command client
 * @when ListTorii is called
 * @then the same method of the server is called and client successfully return
 */
TEST_F(SyncClient, ListTorii) {
  iroha::protocol::TxList tx;
  EXPECT_CALL(*server, ListTorii(_, _, _)).WillOnce(Return(grpc::Status()));
  torii::CommandSyncClient client(ip, port);
  auto stat = client.ListTorii(tx);
  ASSERT_TRUE(stat.ok());
}

/**
 * @given command client
 * @when StatusStream is called
 * @then the same method of the server is called and client successfully return
 */
TEST_F(SyncClient, StatusStream) {
  iroha::protocol::TxStatusRequest tx;
  iroha::protocol::ToriiResponse resp;
  resp.set_tx_hash(std::string(kHashLength, '1'));
  std::vector<iroha::protocol::ToriiResponse> responses;
  EXPECT_CALL(*server, StatusStream(_, _, _))
      .WillOnce(Invoke([&](auto,
                           auto,
                           grpc::ServerWriter<iroha::protocol::ToriiResponse>
                               *response_writer) {
        response_writer->Write(resp);
        return grpc::Status();
      }));
  torii::CommandSyncClient client(ip, port);
  client.StatusStream(tx, responses);
  ASSERT_EQ(responses.size(), 1);
  ASSERT_EQ(responses[0].tx_hash(), resp.tx_hash());
}
