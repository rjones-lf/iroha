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

#include <responses.pb.h>

#include <endpoint.pb.h>

#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

#include "client.hpp"

#include "main/server_runner.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii/query_service.hpp"

#include "model/converters/json_common.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/permissions.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::ametsuchi;
using namespace iroha::network;
using namespace iroha::validation;
using namespace iroha::model::converters;
using namespace iroha::model;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds proposal_delay = 10s;

class ClientServerTest : public testing::Test {
 public:
  virtual void SetUp() {
    spdlog::set_level(spdlog::level::off);
    // Run a server
    runner = std::make_unique<ServerRunner>(std::string(Ip) + ":"
                                            + std::to_string(Port));
    th = std::thread([this] {
      // ----------- Command Service --------------
      pcsMock = std::make_shared<MockPeerCommunicationService>();
      svMock = std::make_shared<MockStatelessValidator>();
      wsv_query = std::make_shared<MockWsvQuery>();
      block_query = std::make_shared<MockBlockQuery>();
      storageMock = std::make_shared<MockStorage>();

      rxcpp::subjects::subject<iroha::model::Proposal> prop_notifier;
      rxcpp::subjects::subject<Commit> commit_notifier;

      EXPECT_CALL(*pcsMock, on_proposal())
          .WillRepeatedly(Return(prop_notifier.get_observable()));

      EXPECT_CALL(*pcsMock, on_commit())
          .WillRepeatedly(Return(commit_notifier.get_observable()));

      auto tx_processor =
          std::make_shared<iroha::torii::TransactionProcessorImpl>(pcsMock,
                                                                   svMock);
      auto pb_tx_factory =
          std::make_shared<iroha::model::converters::PbTransactionFactory>();

      //----------- Query Service ----------
      auto qpf = std::make_unique<iroha::model::QueryProcessingFactory>(
          wsv_query, block_query);

      auto qpi = std::make_shared<iroha::torii::QueryProcessorImpl>(
          std::move(qpf), svMock);

      auto pb_query_factory =
          std::make_shared<iroha::model::converters::PbQueryFactory>();
      auto pb_query_resp_factory =
          std::make_shared<iroha::model::converters::PbQueryResponseFactory>();

      //----------- Server run ----------------
      runner
          ->append(std::make_unique<torii::CommandService>(
              pb_tx_factory, tx_processor, storageMock, proposal_delay))
          .append(std::make_unique<torii::QueryService>(
              pb_query_factory, pb_query_resp_factory, qpi))
          .run();
    });

    runner->waitForServersReady();
  }

  virtual void TearDown() {
    runner->shutdown();
    th.join();
  }

  std::unique_ptr<ServerRunner> runner;
  std::thread th;
  std::shared_ptr<MockPeerCommunicationService> pcsMock;
  std::shared_ptr<MockStatelessValidator> svMock;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
  std::shared_ptr<MockStorage> storageMock;
};

TEST_F(ClientServerTest, SendTxWhenValid) {
  iroha_cli::CliClient client(Ip, Port);
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Transaction &>()))
      .WillOnce(Return(true));
  EXPECT_CALL(*pcsMock, propagate_transaction(_)).Times(1);

  auto json_string =
      R"({"signatures": [ {
            "pubkey":
              "2323232323232323232323232323232323232323232323232323232323232323",
            "signature":
              "23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323"
        }], "created_ts": 0,
          "creator_account_id": "123",
          "tx_counter": 0,
          "commands": [{
            "command_type": "AddPeer",
            "peer": {
              "address": "localhost",
              "peer_key": "2323232323232323232323232323232323232323232323232323232323232323"
            }
        }]})";

  JsonTransactionFactory tx_factory;
  auto json_doc = stringToJson(json_string);
  ASSERT_TRUE(json_doc.has_value());
  auto model_tx = tx_factory.deserialize(json_doc.value());
  ASSERT_TRUE(model_tx.has_value());
  auto status = client.sendTx(model_tx.value());
  ASSERT_EQ(status.answer, iroha_cli::CliClient::OK);
}

TEST_F(ClientServerTest, SendTxWhenInvalidJson) {
  iroha_cli::CliClient client(Ip, Port);
  // Must not call stateful validation since json is invalid
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Transaction &>()))
      .Times(0);
  // Json with no Transaction
  auto json_string =
      R"({"creator_account_id": "test",
          "commands":[{
            "command_type": "AddPeer",
            "peer": {
              "address": "localhost",
              "peer_key": "2323232323232323232323232323232323232323232323232323232323232323"
            }
          }]
        })";
  JsonTransactionFactory tx_factory;
  auto json_doc = stringToJson(json_string);
  ASSERT_TRUE(json_doc.has_value());
  auto model_tx = tx_factory.deserialize(json_doc.value());
  ASSERT_FALSE(model_tx.has_value());
}

TEST_F(ClientServerTest, SendTxWhenStatelessInvalid) {
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Transaction &>()))
      .WillOnce(Return(false));
  auto json_string =
      R"({"signatures": [{
            "pubkey":
              "2423232323232323232323232323232323232323232323232323232323232323",
            "signature":
              "23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323"
        }], "created_ts": 0,
          "creator_account_id": "123",
          "tx_counter": 0,
          "commands": [{
            "command_type": "AddPeer",
              "peer": {
                "address": "localhost",
                "peer_key": "2323232323232323232323232323232323232323232323232323232323232323"
              }
        }]})";

  auto doc = iroha::model::converters::stringToJson(json_string).value();
  iroha::model::converters::JsonTransactionFactory transactionFactory;
  auto tx = transactionFactory.deserialize(doc).value();

  ASSERT_EQ(iroha_cli::CliClient(Ip, Port).sendTx(tx).answer,
            iroha_cli::CliClient::OK);
  ASSERT_EQ(iroha_cli::CliClient(Ip, Port)
                .getTxStatus(iroha::hash(tx).to_string())
                .answer.tx_status(),
            iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
}

TEST_F(ClientServerTest, SendQueryWhenInvalidJson) {
  iroha_cli::CliClient client(Ip, Port);
  // Must not call stateful validation since json is invalid and shouldn't be
  // passed to stateless validation
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Query &>())).Times(0);

  auto json_query =
      R"({"creator_account_id": "test",
          "commands":[{
            "command_type": "AddPeer",
            "peer": {
              "address": "localhost",
              "peer_key": "2323232323232323232323232323232323232323232323232323232323232323"
            }
          }]
        })";

  JsonQueryFactory queryFactory;
  auto model_query = queryFactory.deserialize(json_query);
  ASSERT_FALSE(model_query.has_value());
}

TEST_F(ClientServerTest, SendQueryWhenStatelessInvalid) {
  iroha_cli::CliClient client(Ip, Port);
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(false));

  auto json_query =
      R"({"signature": {
            "pubkey":
              "2323232323232323232323232323232323232323232323232323232323232323",
            "signature":
              "23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323"
          },
          "created_ts": 0,
          "creator_account_id": "123",
          "query_counter": 0,
          "query_type": "GetAccount",
          "account_id": "test@test"
        })";

  JsonQueryFactory queryFactory;
  auto model_query = queryFactory.deserialize(json_query);
  ASSERT_TRUE(model_query.has_value());

  auto res = client.sendQuery(model_query.value());
  ASSERT_TRUE(res.status.ok());
  ASSERT_TRUE(res.answer.has_error_response());
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::model::ErrorResponse::STATELESS_INVALID);
}

TEST_F(ClientServerTest, SendQueryWhenValid) {
  iroha_cli::CliClient client(Ip, Port);
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));
  auto account_admin = iroha::model::Account();
  account_admin.account_id = "admin@test";

  auto account_test = iroha::model::Account();
  account_test.account_id = "test@test";

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  "admin@test", "test@test", can_get_my_account))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccount("test@test"))
      .WillOnce(Return(account_test));

  std::vector<std::string> roles = {"user"};
  EXPECT_CALL(*wsv_query, getAccountRoles("test@test")).WillOnce(Return(roles));
  EXPECT_CALL(*wsv_query, getAccountRoles("admin@test"))
      .WillOnce(Return(nonstd::nullopt));

  auto json_query =
      R"({"signature": {
            "pubkey":
              "2323232323232323232323232323232323232323232323232323232323232323",
            "signature":
              "23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323"
          },
          "created_ts": 0,
          "creator_account_id": "admin@test",
          "query_counter": 0,
          "query_type": "GetAccount",
          "account_id": "test@test"
        })";

  JsonQueryFactory queryFactory;
  auto model_query = queryFactory.deserialize(json_query);
  ASSERT_TRUE(model_query.has_value());

  auto res = client.sendQuery(model_query.value());
  ASSERT_EQ(res.answer.account_response().account().account_id(), "test@test");
}

TEST_F(ClientServerTest, SendQueryWhenStatefulInvalid) {
  iroha_cli::CliClient client(Ip, Port);
  EXPECT_CALL(*svMock, validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));
  auto account_admin = iroha::model::Account();
  account_admin.account_id = "admin@test";

  auto account_test = iroha::model::Account();
  account_test.account_id = "test@test";

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  "admin@test", "test@test", can_get_my_account))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccount("test@test")).Times(0);

  auto json_query =
      R"({"signature": {
            "pubkey":
              "2323232323232323232323232323232323232323232323232323232323232323",
            "signature":
              "23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323"
          },
          "created_ts": 0,
          "creator_account_id": "admin@test",
          "query_counter": 0,
          "query_type": "GetAccount",
          "account_id": "test@test"
        })";

  JsonQueryFactory queryFactory;
  auto model_query = queryFactory.deserialize(json_query);
  ASSERT_TRUE(model_query.has_value());

  auto res = client.sendQuery(model_query.value());
  ASSERT_TRUE(res.status.ok());
  ASSERT_TRUE(res.answer.has_error_response());
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::protocol::ErrorResponse::STATEFUL_INVALID);
}
