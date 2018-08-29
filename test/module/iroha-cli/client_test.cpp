/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/proposal.hpp"
#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/pending_txs_storage/pending_txs_storage_mock.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

#include "client.hpp"

#include "execution/query_execution_impl.hpp"
#include "main/server_runner.hpp"
#include "torii/command_service.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii/query_service.hpp"

#include "model/converters/json_common.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"

#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::ametsuchi;
using namespace iroha::network;
using namespace iroha::validation;
using namespace shared_model::proto;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds initial_timeout = 1s;
constexpr std::chrono::milliseconds nonfinal_timeout = 2 * 10s;

/**
Here we imitate the behavior of StatusStram client but on a bit lower level. So
the do-while cycle imitates client resubscription to the stream. Stream
"expiration" is a valid designed case (see pr #1615 for the details).

The number of attempts (3) is a magic constant here. The idea behind this number
is the following: only one resubscription is usually enough to pass the test; if
three resubscribes were not enough, then most likely there is another bug.
 */
constexpr uint32_t status_read_attempts = 3;

class ClientServerTest : public testing::Test {
 public:
  virtual void SetUp() {
    spdlog::set_level(spdlog::level::off);
    // Run a server
    runner = std::make_unique<ServerRunner>(ip + ":0");

    // ----------- Command Service --------------
    pcsMock = std::make_shared<MockPeerCommunicationService>();
    mst = std::make_shared<iroha::MockMstProcessor>();
    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();
    storage = std::make_shared<MockStorage>();

    rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
        prop_notifier;
    rxcpp::subjects::subject<iroha::synchronizer::SynchronizationEvent>
        commit_notifier;
    EXPECT_CALL(*pcsMock, on_proposal())
        .WillRepeatedly(Return(prop_notifier.get_observable()));
    EXPECT_CALL(*pcsMock, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));
    EXPECT_CALL(*pcsMock, on_verified_proposal())
        .WillRepeatedly(Return(verified_prop_notifier.get_observable()));

    EXPECT_CALL(*mst, onPreparedBatchesImpl())
        .WillRepeatedly(Return(mst_prepared_notifier.get_observable()));
    EXPECT_CALL(*mst, onExpiredBatchesImpl())
        .WillRepeatedly(Return(mst_expired_notifier.get_observable()));

    auto status_bus = std::make_shared<iroha::torii::StatusBusImpl>();
    auto tx_processor =
        std::make_shared<iroha::torii::TransactionProcessorImpl>(
            pcsMock, mst, status_bus);

    auto pb_tx_factory =
        std::make_shared<iroha::model::converters::PbTransactionFactory>();

    auto pending_txs_storage =
        std::make_shared<iroha::MockPendingTransactionStorage>();

    //----------- Query Service ----------
    EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_query));
    EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_query));

    auto qpi = std::make_shared<iroha::torii::QueryProcessorImpl>(
        storage,
        std::make_shared<iroha::QueryExecutionImpl>(storage,
                                                    pending_txs_storage));

    //----------- Server run ----------------
    runner
        ->append(std::make_unique<torii::CommandService>(tx_processor,
                                                         storage,
                                                         status_bus,
                                                         initial_timeout,
                                                         nonfinal_timeout))
        .append(std::make_unique<torii::QueryService>(qpi))
        .run()
        .match(
            [this](iroha::expected::Value<int> port) {
              this->port = port.value;
            },
            [](iroha::expected::Error<std::string> err) {
              FAIL() << err.error;
            });

    runner->waitForServersReady();
  }
  decltype(shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair())
      pair =
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  std::vector<shared_model::interface::types::PubkeyType> signatories = {
      pair.publicKey()};

  std::unique_ptr<ServerRunner> runner;
  std::shared_ptr<MockPeerCommunicationService> pcsMock;
  std::shared_ptr<iroha::MockMstProcessor> mst;

  rxcpp::subjects::subject<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      verified_prop_notifier;
  rxcpp::subjects::subject<iroha::DataType> mst_prepared_notifier;
  rxcpp::subjects::subject<iroha::DataType> mst_expired_notifier;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
  std::shared_ptr<MockStorage> storage;

  const std::string ip = "127.0.0.1";
  int port;
};

TEST_F(ClientServerTest, SendTxWhenValid) {
  iroha_cli::CliClient client(ip, port);
  EXPECT_CALL(*pcsMock, propagate_batch(_)).Times(1);

  auto shm_tx = shared_model::proto::TransactionBuilder()
                    .creatorAccountId("some@account")
                    .createdTime(iroha::time::now())
                    .setAccountQuorum("some@account", 2)
                    .quorum(1)
                    .build()
                    .signAndAddSignature(
                        shared_model::crypto::DefaultCryptoAlgorithmType::
                            generateKeypair())
                    .finish();

  auto status = client.sendTx(shm_tx);
  ASSERT_EQ(status.answer, iroha_cli::CliClient::OK);
}

TEST_F(ClientServerTest, SendTxWhenInvalidJson) {
  iroha_cli::CliClient client(ip, port);
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
  iroha::model::converters::JsonTransactionFactory tx_factory;
  auto json_doc = iroha::model::converters::stringToJson(json_string);
  ASSERT_TRUE(json_doc);
  auto model_tx = tx_factory.deserialize(json_doc.value());
  ASSERT_FALSE(model_tx);
}

TEST_F(ClientServerTest, SendTxWhenStatelessInvalid) {
  // creating stateless invalid tx
  auto shm_tx = TestTransactionBuilder()
                    .creatorAccountId("some@account")
                    .createdTime(iroha::time::now())
                    .setAccountQuorum("some@@account", 2)
                    .build();

  ASSERT_EQ(iroha_cli::CliClient(ip, port).sendTx(shm_tx).answer,
            iroha_cli::CliClient::OK);
  auto getAnswer = [&]() {
    return iroha_cli::CliClient(ip, port)
        .getTxStatus(shared_model::crypto::toBinaryString(shm_tx.hash()))
        .answer;
  };
  decltype(getAnswer()) answer;
  auto read_attempt_counter(status_read_attempts);
  do {
    answer = getAnswer();
  } while (answer.tx_status()
               != iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED
           and --read_attempt_counter);
  ASSERT_EQ(answer.tx_status(),
            iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
  ASSERT_NE(answer.error_message().size(), 0);
}

/**
 * This test checks, if tx, which did not pass stateful validation, is shown to
 * client with a corresponding status and error message
 *
 * @given real client and mocked pcs
 * @when sending a stateless valid transaction @and failing it at stateful
 * validation
 * @then ensure that client sees:
 *       - status of this transaction as STATEFUL_VALIDATION_FAILED
 *       - error message is the same, as the one with which transaction was
 *         failed
 */
TEST_F(ClientServerTest, SendTxWhenStatefulInvalid) {
  iroha_cli::CliClient client(ip, port);
  EXPECT_CALL(*pcsMock, propagate_batch(_)).Times(1);

  // creating stateful invalid tx
  auto tx = TransactionBuilder()
                .creatorAccountId("some@account")
                .createdTime(iroha::time::now())
                .transferAsset("some@account",
                               "another@account",
                               "doge#doge",
                               "some transfer",
                               "100.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();
  ASSERT_EQ(client.sendTx(tx).answer, iroha_cli::CliClient::OK);

  // fail the tx
  auto verified_proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().height(0).createdTime(iroha::time::now()).build());
  verified_prop_notifier.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(verified_proposal,
                         iroha::validation::TransactionsErrors{std::make_pair(
                             iroha::validation::CommandError{
                                 "CommandName", "CommandError", true, 2},
                             tx.hash())})));
  auto stringified_error = "Stateful validation error in transaction "
                           + tx.hash().hex() + ": command 'CommandName' with "
                                               "index '2' did not pass verification with "
                                               "error 'CommandError'";

  auto getAnswer = [&]() {
    return client.getTxStatus(shared_model::crypto::toBinaryString(tx.hash()))
        .answer;
  };
  decltype(getAnswer()) answer;
  auto read_attempt_counter(status_read_attempts);
  do {
    // check it really failed with specific message
    answer = getAnswer();
  } while (answer.tx_status()
               != iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED
           and --read_attempt_counter);
  ASSERT_EQ(answer.tx_status(),
            iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED);
  ASSERT_EQ(answer.error_message(), stringified_error);
}

TEST_F(ClientServerTest, SendQueryWhenInvalidJson) {
  iroha_cli::CliClient client(ip, port);
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

  iroha::model::converters::JsonQueryFactory queryFactory;
  auto model_query = queryFactory.deserialize(json_query);
  ASSERT_FALSE(model_query);
}

TEST_F(ClientServerTest, SendQueryWhenStatelessInvalid) {
  iroha_cli::CliClient client(ip, port);

  shared_model::proto::Query query = TestQueryBuilder()
                                         .createdTime(0)
                                         .creatorAccountId("123")
                                         .getAccount("asd")
                                         .build();
  auto proto_query = query.getTransport();

  auto res = client.sendQuery(query);
  ASSERT_TRUE(res.status.ok());
  ASSERT_TRUE(res.answer.has_error_response());
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::protocol::ErrorResponse::STATELESS_INVALID);
  ASSERT_NE(res.answer.error_response().message().size(), 0);
}

TEST_F(ClientServerTest, SendQueryWhenValid) {
  // TODO: 30/04/2018 x3medima17, fix Uninteresting mock function call, IR-1187
  iroha_cli::CliClient client(ip, port);

  std::shared_ptr<shared_model::interface::Account> account_test = clone(
      shared_model::proto::AccountBuilder().accountId("test@test").build());

  EXPECT_CALL(*wsv_query, getSignatories("admin@test"))
      .WillRepeatedly(Return(signatories));

  EXPECT_CALL(*wsv_query, getAccountDetail("test@test", "", ""))
      .WillOnce(Return(boost::make_optional(std::string("value"))));

  const std::vector<std::string> kRole{"role"};
  EXPECT_CALL(*wsv_query, getAccountRoles("admin@test"))
      .WillOnce(Return(boost::make_optional(kRole)));
  EXPECT_CALL(*wsv_query, getRolePermissions(kRole[0]))
      .WillOnce(Return(shared_model::interface::RolePermissionSet{
          shared_model::interface::permissions::Role::kGetAllAccDetail}));

  auto query = QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId("admin@test")
                   .queryCounter(1)
                   .getAccountDetail("test@test")
                   .build()
                   .signAndAddSignature(pair)
                   .finish();

  auto res = client.sendQuery(query);
  ASSERT_EQ(res.answer.account_detail_response().detail(), "value");
}

TEST_F(ClientServerTest, SendQueryWhenStatefulInvalid) {
  iroha_cli::CliClient client(ip, port);

  EXPECT_CALL(*wsv_query, getSignatories("admin@test"))
      .WillRepeatedly(Return(signatories));

  EXPECT_CALL(*wsv_query, getAccountRoles("admin@test"))
      .WillOnce(Return(boost::none));

  auto query = QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId("admin@test")
                   .queryCounter(1)
                   .getAccountDetail("test@test")
                   .build()
                   .signAndAddSignature(pair)
                   .finish();

  auto res = client.sendQuery(query);
  ASSERT_EQ(res.answer.error_response().reason(),
            iroha::protocol::ErrorResponse::STATEFUL_INVALID);
}
