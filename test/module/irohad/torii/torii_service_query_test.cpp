/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>

#include "backend/protobuf/proto_query_response_factory.hpp"
#include "backend/protobuf/proto_transport_factory.hpp"
#include "backend/protobuf/query_responses/proto_block_query_response.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "builders/default_builders.hpp"
#include "builders/protobuf/queries.hpp"
#include "main/server_runner.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "torii/query_client.hpp"
#include "torii/query_service.hpp"
#include "validators/protobuf/proto_query_validator.hpp"

using ::testing::_;
using ::testing::A;
using ::testing::An;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Truly;

/**
 * Module tests on torii query service
 */
class ToriiQueryServiceTest : public ::testing::Test {
 public:
  virtual void SetUp() {
    runner = std::make_unique<ServerRunner>(ip + ":0");

    // ----------- Command Service --------------
    query_processor = std::make_shared<iroha::torii::MockQueryProcessor>();

    //----------- Server run ----------------
    initQueryFactory();
    runner
        ->append(std::make_unique<torii::QueryService>(query_processor,
                                                       query_factory))
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

  void initQueryFactory() {
    std::unique_ptr<shared_model::validation::AbstractValidator<
        shared_model::interface::Query>>
        query_validator = std::make_unique<
            shared_model::validation::DefaultSignedQueryValidator>();
    std::unique_ptr<
        shared_model::validation::AbstractValidator<iroha::protocol::Query>>
        proto_query_validator =
            std::make_unique<shared_model::validation::ProtoQueryValidator>();
    query_factory = std::make_shared<shared_model::proto::ProtoTransportFactory<
        shared_model::interface::Query,
        shared_model::proto::Query>>(std::move(query_validator),
                                     std::move(proto_query_validator));
  }

  std::unique_ptr<ServerRunner> runner;
  std::shared_ptr<iroha::torii::MockQueryProcessor> query_processor;
  torii::QueryService::QueryFactoryType query_factory;

  iroha::protocol::Block block;

  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

  const std::string ip = "127.0.0.1";
  int port;
};

/**
 * @given valid blocks query
 * @when blocks query is executed
 * @then valid blocks response is received and contains block emitted by query
 * processor
 */
TEST_F(ToriiQueryServiceTest, FetchBlocksWhenValidQuery) {
  auto blocks_query = std::make_shared<shared_model::proto::BlocksQuery>(
      shared_model::proto::BlocksQueryBuilder()
          .creatorAccountId("user@domain")
          .createdTime(iroha::time::now())
          .queryCounter(1)
          .build()
          .signAndAddSignature(
              shared_model::crypto::DefaultCryptoAlgorithmType::
                  generateKeypair())
          .finish());

  iroha::protocol::Block block;
  block.mutable_payload()->set_height(123);
  auto proto_block = std::make_unique<shared_model::proto::Block>(block);
  std::shared_ptr<shared_model::interface::BlockQueryResponse> block_response =
      shared_model::proto::ProtoQueryResponseFactory().createBlockQueryResponse(
          std::move(proto_block));

  EXPECT_CALL(*query_processor,
              blocksQueryHandle(Truly([&blocks_query](auto &query) {
                return query == *blocks_query;
              })))
      .WillOnce(Return(rxcpp::observable<>::just(block_response)));

  auto client = torii_utils::QuerySyncClient(ip, port);
  auto proto_blocks_query =
      std::static_pointer_cast<shared_model::proto::BlocksQuery>(blocks_query);
  auto responses = client.FetchCommits(proto_blocks_query->getTransport());

  ASSERT_EQ(responses.size(), 1);
  auto response = responses.at(0);
  ASSERT_TRUE(response.has_block_response());
  ASSERT_EQ(response.block_response().block().SerializeAsString(),
            block.SerializeAsString());
}

/**
 * @given stateless invalid blocks query
 * @when blocks query is executed
 * @then block error response is received
 */
TEST_F(ToriiQueryServiceTest, FetchBlocksWhenInvalidQuery) {
  EXPECT_CALL(*query_processor, blocksQueryHandle(_)).Times(0);

  auto blocks_query = std::make_shared<shared_model::proto::BlocksQuery>(
      TestUnsignedBlocksQueryBuilder()
          .creatorAccountId("asd@@domain")  // invalid account id name
          .createdTime(iroha::time::now())
          .queryCounter(1)
          .build()
          .signAndAddSignature(
              shared_model::crypto::DefaultCryptoAlgorithmType::
                  generateKeypair())
          .finish());

  auto client = torii_utils::QuerySyncClient(ip, port);
  auto proto_blocks_query =
      std::static_pointer_cast<shared_model::proto::BlocksQuery>(blocks_query);
  auto responses = client.FetchCommits(proto_blocks_query->getTransport());

  ASSERT_EQ(responses.size(), 1);
  auto response = responses.at(0);
  ASSERT_TRUE(response.has_block_error_response());
}
