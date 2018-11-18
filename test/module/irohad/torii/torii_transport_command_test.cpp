/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <iterator>
#include <string>

#include "backend/protobuf/proto_transport_factory.hpp"
#include "backend/protobuf/proto_tx_status_factory.hpp"
#include "builders/protobuf/transaction.hpp"
#include "endpoint.pb.h"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/vendor/grpc_mocks.hpp"
#include "torii/impl/command_service_impl.hpp"
#include "torii/impl/command_service_transport_grpc.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

constexpr size_t kTimes = 5;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

using namespace iroha::network;
using namespace iroha::ametsuchi;
using namespace iroha::torii;
using namespace iroha::synchronizer;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds initial_timeout = 1s;
constexpr std::chrono::milliseconds nonfinal_timeout = 2 * 10s;

class ToriiTransportCommandTest : public testing::Test {
 public:
  void SetUp() override {
    block_query = std::make_shared<MockBlockQuery>();
    storage = std::make_shared<MockStorage>();

    EXPECT_CALL(*block_query, getTxByHashSync(_))
        .WillRepeatedly(Return(boost::none));
    EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_query));

    status_bus = std::make_shared<MockStatusBus>();

    status_factory =
        std::make_shared<shared_model::proto::ProtoTxStatusFactory>();
    std::unique_ptr<shared_model::validation::AbstractValidator<
        shared_model::interface::Transaction>>
        transaction_validator = std::make_unique<
            shared_model::validation::DefaultUnsignedTransactionValidator>();
    auto transaction_factory =
        std::make_shared<shared_model::proto::ProtoTransportFactory<
            shared_model::interface::Transaction,
            shared_model::proto::Transaction>>(
            std::move(transaction_validator));
    batch_parser =
        std::make_shared<shared_model::interface::TransactionBatchParserImpl>();
    auto batch_factory = std::make_shared<
        shared_model::interface::TransactionBatchFactoryImpl>();
    command_service = std::make_shared<MockCommandService>();
    transport_grpc = std::make_shared<torii::CommandServiceTransportGrpc>(
        command_service,
        status_bus,
        initial_timeout,
        nonfinal_timeout,
        status_factory,
        transaction_factory,
        batch_parser,
        batch_factory);
  }

  std::shared_ptr<MockBlockQuery> block_query;
  std::shared_ptr<MockStorage> storage;

  std::shared_ptr<MockStatusBus> status_bus;

  std::shared_ptr<shared_model::interface::TransactionBatchParser> batch_parser;

  std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory;

  std::shared_ptr<MockCommandService> command_service;
  std::shared_ptr<torii::CommandServiceTransportGrpc> transport_grpc;
};

/**
 * @given torii service and number of transactions
 * @when retrieving their status
 * @then ensure those are coming from CommandService
 */
TEST_F(ToriiTransportCommandTest, Status) {
  for (size_t i = 0; i < kTimes; ++i) {
    grpc::ServerContext context;

    iroha::protocol::TxStatusRequest tx_request;
    const auto hash = TestTransactionBuilder()
                          .creatorAccountId("account" + std::to_string(i))
                          .build()
                          .hash();
    tx_request.set_tx_hash(shared_model::crypto::toBinaryString(hash));

    iroha::protocol::ToriiResponse toriiResponse;
    std::shared_ptr<shared_model::interface::TransactionResponse> response =
        status_factory->makeEnoughSignaturesCollected(hash, {});

    EXPECT_CALL(*command_service, getStatus(hash)).WillOnce(Return(response));

    transport_grpc->Status(&context, &tx_request, &toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::ENOUGH_SIGNATURES_COLLECTED);
  }
}

/**
 * @given torii service and number of transactions
 * @when calling ListTorii
 * @then ensure that CommandService called handleTransactionBatch as the tx num
 */
TEST_F(ToriiTransportCommandTest, ListTorii) {
  grpc::ServerContext context;
  google::protobuf::Empty response;

  iroha::protocol::TxList request;
  std::vector<shared_model::interface::types::HashType> tx_hashes;
  for (size_t i = 0; i < kTimes; ++i) {
    auto shm_tx = shared_model::proto::TransactionBuilder()
                      .creatorAccountId("doge@master" + std::to_string(i))
                      .createdTime(iroha::time::now())
                      .setAccountQuorum("doge@master", 2)
                      .quorum(1)
                      .build()
                      .signAndAddSignature(
                          shared_model::crypto::DefaultCryptoAlgorithmType::
                              generateKeypair())
                      .finish();
    tx_hashes.push_back(shm_tx.hash());
    new (request.add_transactions())
        iroha::protocol::Transaction(shm_tx.getTransport());
  }

  EXPECT_CALL(*command_service, handleTransactionBatch(_)).Times(kTimes);
  transport_grpc->ListTorii(&context, &request, &response);
}

/**
 * @given torii service and number of invalid transactions
 * @when calling ListTorii
 * @then ensure that CommandService haven't called handleTransactionBatch
 *       and StatusBus update status tx num times
 */
TEST_F(ToriiTransportCommandTest, ListToriiInvalid) {
  grpc::ServerContext context;
  google::protobuf::Empty response;

  iroha::protocol::TxList request;
  std::vector<shared_model::interface::types::HashType> tx_hashes;
  for (size_t i = 0; i < kTimes; ++i) {
    auto shm_tx = TestTransactionBuilder().build();
    tx_hashes.push_back(shm_tx.hash());
    new (request.add_transactions())
        iroha::protocol::Transaction(shm_tx.getTransport());
  }

  EXPECT_CALL(*command_service, handleTransactionBatch(_)).Times(0);
  EXPECT_CALL(*status_bus, publish(_)).Times(kTimes);

  transport_grpc->ListTorii(&context, &request, &response);
}

/**
 * @given torii service
 *        and some number of valid transactions
 *        and one stateless invalid tx
 * @when calling ListTorii
 * @then ensure that CommandService haven't called handleTransactionBatch
 *       and StatusBus publishes statelessInvalid for all txes
 */
TEST_F(ToriiTransportCommandTest, ListToriiPartialInvalid) {
  grpc::ServerContext context;
  google::protobuf::Empty response;

  iroha::protocol::TxList request;
  for (size_t i = 0; i < kTimes - 1; ++i) {
    auto shm_tx = shared_model::proto::TransactionBuilder()
                      .creatorAccountId("doge@master" + std::to_string(i))
                      .createdTime(iroha::time::now())
                      .setAccountQuorum("doge@master", 2)
                      .quorum(1)
                      .build()
                      .signAndAddSignature(
                          shared_model::crypto::DefaultCryptoAlgorithmType::
                              generateKeypair())
                      .finish();
    new (request.add_transactions())
        iroha::protocol::Transaction(shm_tx.getTransport());
  }
  auto shm_tx = TestTransactionBuilder().build();
  new (request.add_transactions())
      iroha::protocol::Transaction(shm_tx.getTransport());

  EXPECT_CALL(*command_service, handleTransactionBatch(_)).Times(4);
  EXPECT_CALL(*status_bus, publish(_)).WillOnce(Invoke([](auto status) {
    EXPECT_FALSE(status->statelessErrorOrCommandName().empty());
  }));

  transport_grpc->ListTorii(&context, &request, &response);
}

/**
 * @given torii service and command_service with empty status stream
 * @when calling StatusStream on transport
 * @then Ok status is eventually returned without any fault
 */
TEST_F(ToriiTransportCommandTest, StatusStreamEmpty) {
  grpc::ServerContext context;
  iroha::protocol::TxStatusRequest request;

  EXPECT_CALL(*command_service, getStatusStream(_))
      .WillOnce(Return(rxcpp::observable<>::empty<std::shared_ptr<
                           shared_model::interface::TransactionResponse>>()));

  ASSERT_TRUE(transport_grpc->StatusStream(&context, &request, nullptr).ok());
}

/**
 * @given torii service and number of invalid transactions
 * @when calling StatusStream
 * @then ServerWriter call Write method the same number of times
 */
TEST_F(ToriiTransportCommandTest, StatusStream) {
  grpc::ServerContext context;
  iroha::protocol::TxStatusRequest request;
  iroha::MockServerWriter<iroha::protocol::ToriiResponse> response_writer;

  std::vector<std::shared_ptr<shared_model::interface::TransactionResponse>>
      responses;
  for (size_t i = 0; i < kTimes; ++i) {
    auto hash = TestTransactionBuilder().build().hash();
    auto push_response = [this, /*i, */ hash = std::move(hash), &responses](
                             auto member_fn) {
      responses.emplace_back((this->status_factory.get()->*member_fn)(
                                 hash, {} /*"ErrMessage" + std::to_string(i)*/)
                                 .release());
    };

    using shared_model::interface::TxStatusFactory;
    // cover different type of statuses
    switch (i) {
      case 0:
        push_response(&TxStatusFactory::makeStatelessFail);
        break;
      case 1:
        push_response(&TxStatusFactory::makeStatelessValid);
        break;
      case 2:
        push_response(&TxStatusFactory::makeStatefulFail);
        break;
      case 3:
        push_response(&TxStatusFactory::makeStatefulValid);
        break;
      case 4:
        push_response(&TxStatusFactory::makeCommitted);
        break;
      default:
        throw std::runtime_error("Out of range");
    }
  }

  EXPECT_CALL(*command_service, getStatusStream(_))
      .WillOnce(Return(rxcpp::observable<>::iterate(responses)));
  EXPECT_CALL(response_writer, Write(_, _))
      .Times(kTimes)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(transport_grpc
                  ->StatusStream(
                      &context,
                      &request,
                      reinterpret_cast<
                          grpc::ServerWriter<iroha::protocol::ToriiResponse> *>(
                          &response_writer))
                  .ok());
}
