/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

#include "backend/protobuf/proto_transport_factory.hpp"
#include "backend/protobuf/proto_tx_status_factory.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"
#include "endpoint.pb.h"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/shared_model/interface/mock_transaction_batch_factory.hpp"
#include "module/shared_model/validators/validators.hpp"
#include "module/vendor/grpc_mocks.hpp"
#include "torii/impl/command_service_impl.hpp"
#include "torii/impl/command_service_transport_grpc.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "validators/protobuf/proto_transaction_validator.hpp"

constexpr size_t kTimes = 5;

using ::testing::_;
using ::testing::A;
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
 private:
  using TransportFactory = shared_model::proto::ProtoTransportFactory<
      shared_model::interface::Transaction,
      shared_model::proto::Transaction>;
  using TransportFactoryBase =
      shared_model::interface::AbstractTransportFactory<
          shared_model::interface::Transaction,
          shared_model::proto::Transaction::TransportType>;
  using TxValidator = shared_model::validation::MockValidator<
      shared_model::interface::Transaction>;
  using ProtoTxValidator =
      shared_model::validation::MockValidator<iroha::protocol::Transaction>;

 public:
  /**
   * Initialize factory dependencies
   */
  void init() {
    status_factory =
        std::make_shared<shared_model::proto::ProtoTxStatusFactory>();

    auto validator = std::make_unique<TxValidator>();
    tx_validator = validator.get();
    auto proto_validator = std::make_unique<ProtoTxValidator>();
    proto_tx_validator = proto_validator.get();
    transaction_factory = std::make_shared<TransportFactory>(
        std::move(validator), std::move(proto_validator));

    batch_parser =
        std::make_shared<shared_model::interface::TransactionBatchParserImpl>();
    batch_factory = std::make_shared<MockTransactionBatchFactory>();
  }

  void SetUp() override {
    init();

    status_bus = std::make_shared<MockStatusBus>();
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

  std::shared_ptr<MockStatusBus> status_bus;
  const TxValidator *tx_validator;
  const ProtoTxValidator *proto_tx_validator;

  std::shared_ptr<TransportFactoryBase> transaction_factory;
  std::shared_ptr<shared_model::interface::TransactionBatchParser> batch_parser;
  std::shared_ptr<MockTransactionBatchFactory> batch_factory;

  std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory;

  std::shared_ptr<MockCommandService> command_service;
  std::shared_ptr<torii::CommandServiceTransportGrpc> transport_grpc;

  const size_t kHashLength =
      shared_model::crypto::CryptoProviderEd25519Sha3::kHashLength;
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
    const shared_model::crypto::Hash hash(std::string(kHashLength, '1'));
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
  for (size_t i = 0; i < kTimes; ++i) {
    request.add_transactions();
  }

  EXPECT_CALL(*proto_tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Return(shared_model::validation::Answer{}));
  EXPECT_CALL(*tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Return(shared_model::validation::Answer{}));
  EXPECT_CALL(
      *batch_factory,
      createTransactionBatch(
          A<const shared_model::interface::types::SharedTxsCollectionType &>()))
      .Times(kTimes);

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
  for (size_t i = 0; i < kTimes; ++i) {
    request.add_transactions();
  }

  shared_model::validation::Answer error;
  error.addReason(std::make_pair("some error", std::vector<std::string>{}));
  EXPECT_CALL(*proto_tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Return(shared_model::validation::Answer{}));
  EXPECT_CALL(*tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Return(error));
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

  iroha::protocol::TxList request{};
  for (size_t i = 0; i < kTimes; ++i) {
    request.add_transactions();
  }

  int counter = 0;
  EXPECT_CALL(*proto_tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Return(shared_model::validation::Answer{}));
  EXPECT_CALL(*tx_validator, validate(_))
      .Times(kTimes)
      .WillRepeatedly(Invoke([&counter](const auto &) mutable {
        shared_model::validation::Answer res;
        if (counter++ == kTimes - 1) {
          res.addReason(
              std::make_pair("some error", std::vector<std::string>{}));
        }
        return res;
      }));
  EXPECT_CALL(
      *batch_factory,
      createTransactionBatch(
          A<const shared_model::interface::types::SharedTxsCollectionType &>()))
      .Times(kTimes - 1);

  EXPECT_CALL(*command_service, handleTransactionBatch(_)).Times(kTimes - 1);
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
    shared_model::crypto::Hash hash(std::to_string(i));
    auto push_response = [this, hash = std::move(hash), &responses](
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
