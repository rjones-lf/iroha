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

#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

#include "client.hpp"

#include "main/server_runner.hpp"
#include "torii/command_service.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii/query_service.hpp"

#include "model/converters/json_common.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"
#include "model/permissions.hpp"

#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;

using namespace iroha::ametsuchi;
using namespace iroha::network;
using namespace iroha::validation;
using namespace iroha::model::converters;
using namespace iroha::model;
using namespace shared_model::proto;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds proposal_delay = 10s;

class ClientServerTest : public testing::Test {
 public:
  virtual void SetUp() {
    spdlog::set_level(spdlog::level::off);
    // Run a server
    runner = std::make_unique<ServerRunner>(std::string(Ip) + ":"
                                            + std::to_string(Port));

    // ----------- Command Service --------------
    pcsMock = std::make_shared<MockPeerCommunicationService>();
    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();

    rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
        prop_notifier;
    rxcpp::subjects::subject<iroha::Commit> commit_notifier;

    EXPECT_CALL(*pcsMock, on_proposal())
        .WillRepeatedly(Return(prop_notifier.get_observable()));

    EXPECT_CALL(*pcsMock, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));

    auto tx_processor =
        std::make_shared<iroha::torii::TransactionProcessorImpl>(pcsMock);

    auto pb_tx_factory =
        std::make_shared<iroha::model::converters::PbTransactionFactory>();

    //----------- Query Service ----------
    auto qpf = std::make_unique<iroha::model::QueryProcessingFactory>(
        wsv_query, block_query);

    auto qpi =
        std::make_shared<iroha::torii::QueryProcessorImpl>(std::move(qpf));

    //----------- Server run ----------------
    runner
        ->append(std::make_unique<torii::CommandService>(
            tx_processor, block_query, proposal_delay))
        .append(std::make_unique<torii::QueryService>(qpi))
        .run();

    runner->waitForServersReady();
  }

  std::unique_ptr<ServerRunner> runner;
  std::shared_ptr<MockPeerCommunicationService> pcsMock;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
};

TEST_F(ClientServerTest, SendTxWhenValid) {
  iroha_cli::CliClient client(Ip, Port);
  EXPECT_CALL(*pcsMock, propagate_transaction(_)).Times(1);

  auto shm_tx = shared_model::proto::TransactionBuilder()
                    .creatorAccountId("some@account")
                    .txCounter(1)
                    .createdTime(iroha::time::now())
                    .setAccountQuorum("some@account", 2)
                    .build()
                    .signAndAddSignature(
                        shared_model::crypto::DefaultCryptoAlgorithmType::
                            generateKeypair());

  std::unique_ptr<iroha::model::Transaction> old_model(shm_tx.makeOldModel());
  auto status = client.sendTx(*old_model);
  ASSERT_EQ(status.answer, iroha_cli::CliClient::OK);
}

TEST_F(ClientServerTest, SendTxWhenInvalidJson) {
  iroha_cli::CliClient client(Ip, Port);
  // Must not call stateful validation since json is invalid
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
  ASSERT_TRUE(json_doc);
  auto model_tx = tx_factory.deserialize(json_doc.value());
  ASSERT_FALSE(model_tx);
}

TEST_F(ClientServerTest, SendTxWhenStatelessInvalid) {
  // creating stateless invalid tx
  auto shm_tx = TestTransactionBuilder()
                    .creatorAccountId("some@account")
                    .txCounter(1)
                    .createdTime(iroha::time::now())
                    .setAccountQuorum("some@@account", 2)
                    .build();
  std::unique_ptr<iroha::model::Transaction> old_tx(shm_tx.makeOldModel());

  ASSERT_EQ(iroha_cli::CliClient(Ip, Port).sendTx(*old_tx).answer,
            iroha_cli::CliClient::OK);
  auto tx_hash = shm_tx.hash();
  ASSERT_EQ(iroha_cli::CliClient(Ip, Port)
                .getTxStatus(shared_model::crypto::toBinaryString(tx_hash))
                .answer.tx_status(),
            iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
}

TEST_F(ClientServerTest, SendQueryWhenInvalidJson) {
  iroha_cli::CliClient client(Ip, Port);
  // Must not call stateful validation since json is invalid and shouldn't be
  // passed to stateless validation

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
  ASSERT_FALSE(model_query);
}

TEST_F(ClientServerTest, SendQueryWhenStatelessInvalid) {
  iroha_cli::CliClient client(Ip, Port);

  shared_model::proto::Query query = TestQueryBuilder()
                                         .createdTime(0)
                                         .creatorAccountId("123")
                                         .getAccount("asd")
                                         .build();
  auto proto_query = query.getTransport();

  auto res = client.sendQuery(
      std::shared_ptr<iroha::model::Query>(query.makeOldModel()));
  ASSERT_TRUE(res.status.ok());
  ASSERT_TRUE(res.answer.has_error_response());
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::model::ErrorResponse::STATELESS_INVALID);
}

TEST_F(ClientServerTest, SendQueryWhenValid) {
  iroha_cli::CliClient client(Ip, Port);
  auto account_admin = iroha::model::Account();
  account_admin.account_id = "admin@test";

  std::shared_ptr<shared_model::interface::Account> account_test = clone(
      shared_model::proto::AccountBuilder().accountId("test@test").build());

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  "admin@test", "test@test", can_get_my_acc_detail))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccountDetail("test@test"))
      .WillOnce(Return(boost::make_optional(std::string("value"))));

  auto query = QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId("admin@test")
                   .queryCounter(1)
                   .getAccountDetail("test@test")
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());

  auto res = client.sendQuery(
      std::shared_ptr<iroha::model::Query>(query.makeOldModel()));
  ASSERT_EQ(res.answer.account_detail_response().detail(), "value");
}

TEST_F(ClientServerTest, SendQueryWhenStatefulInvalid) {
  iroha_cli::CliClient client(Ip, Port);
  auto account_admin = iroha::model::Account();
  account_admin.account_id = "admin@test";

  auto account_test = iroha::model::Account();
  account_test.account_id = "test@test";

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  "admin@test", "test@test", can_get_my_acc_detail))
      .WillOnce(Return(false));

  auto query = QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId("admin@test")
                   .queryCounter(1)
                   .getAccountDetail("test@test")
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());

  auto res = client.sendQuery(
      std::shared_ptr<iroha::model::Query>(query.makeOldModel()));
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::protocol::ErrorResponse::STATEFUL_INVALID);
}
