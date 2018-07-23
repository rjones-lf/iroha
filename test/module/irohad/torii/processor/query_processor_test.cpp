/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/query_responses/proto_error_query_response.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/keypair.hpp"
#include "execution/query_execution.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "interfaces/query_responses/block_query_response.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/execution/execution_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_response_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "network/ordering_gate.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "utils/query_error_response_visitor.hpp"
#include "validators/permissions.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::validation;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::A;
using ::testing::Invoke;
using ::testing::Return;

class QueryProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    qry_exec = std::make_shared<MockQueryExecution>();
    storage = std::make_shared<MockStorage>();
    qpi = std::make_shared<torii::QueryProcessorImpl>(storage, qry_exec);
    wsv_queries = std::make_shared<MockWsvQuery>();
    EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
    EXPECT_CALL(*storage, getBlockQuery())
        .WillRepeatedly(Return(block_queries));
  }

  auto getBlocksQuery(const std::string &creator_account_id) {
    return TestUnsignedBlocksQueryBuilder()
        .createdTime(kCreatedTime)
        .creatorAccountId(creator_account_id)
        .queryCounter(kCounter)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  const decltype(iroha::time::now()) kCreatedTime = iroha::time::now();
  const std::string kAccountId = "account@domain";
  const uint64_t kCounter = 1048576;
  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

  std::vector<shared_model::interface::types::PubkeyType> signatories = {
      keypair.publicKey()};
  std::shared_ptr<MockQueryExecution> qry_exec;
  std::shared_ptr<MockWsvQuery> wsv_queries;
  std::shared_ptr<MockBlockQuery> block_queries;
  std::shared_ptr<MockStorage> storage;
  std::shared_ptr<torii::QueryProcessorImpl> qpi;
};

/**
 * @given QueryProcessorImpl and GetAccountDetail query
 * @when queryHandle called at normal flow
 * @then the mocked value of validateAndExecute is returned
 */
TEST_F(QueryProcessorTest, QueryProcessorWhereInvokeInvalidQuery) {
  auto qry = TestUnsignedQueryBuilder()
                 .creatorAccountId(kAccountId)
                 .getAccountDetail(kAccountId)
                 .build()
                 .signAndAddSignature(keypair)
                 .finish();
  auto qry_resp =
      clone(TestQueryResponseBuilder().accountDetailResponse("").build());

  EXPECT_CALL(*wsv_queries, getSignatories(kAccountId))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*qry_exec, validateAndExecute(_))
      .WillOnce(Invoke([&qry_resp](auto &query) { return clone(*qry_resp); }));

  auto response = qpi->queryHandle(qry);
  ASSERT_TRUE(response);
  ASSERT_NO_THROW(boost::apply_visitor(
      framework::SpecifiedVisitor<
          shared_model::interface::AccountDetailResponse>(),
      response->get()));
}

/**
 * @given QueryProcessorImpl and GetAccountDetail query with wrong signature
 * @when queryHandle called at normal flow
 * @then Query Processor returns StatefulFailed response
 */
TEST_F(QueryProcessorTest, QueryProcessorWithWrongKey) {
  auto query = TestUnsignedQueryBuilder()
                   .creatorAccountId(kAccountId)
                   .getAccountDetail(kAccountId)
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair())
                   .finish();
  auto qry_resp =
      clone(TestQueryResponseBuilder().accountDetailResponse("").build());

  EXPECT_CALL(*wsv_queries, getSignatories(kAccountId))
      .WillRepeatedly(Return(signatories));

  auto response = qpi->queryHandle(query);
  ASSERT_TRUE(response);
  ASSERT_NO_THROW(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatefulFailedErrorResponse>(),
      response->get()));
}

/**
 * @given account, ametsuchi queries
 * @when valid block query is send
 * @then Query Processor should start emitting BlockQueryRespones to the
 * observable
 */
TEST_F(QueryProcessorTest, GetBlocksQuery) {
  auto block_number = 5;
  auto block_query = getBlocksQuery(kAccountId);

  EXPECT_CALL(*wsv_queries, getSignatories(kAccountId))
      .WillOnce(Return(signatories));
  EXPECT_CALL(*qry_exec, validate(_)).WillOnce(Return(true));

  auto wrapper = make_test_subscriber<CallExact>(
      qpi->blocksQueryHandle(block_query), block_number);
  wrapper.subscribe([](auto response) {
    ASSERT_NO_THROW({
      boost::apply_visitor(
          framework::SpecifiedVisitor<shared_model::interface::BlockResponse>(),
          response->get());
    });
  });
  for (int i = 0; i < block_number; i++) {
    storage->notifier.get_subscriber().on_next(
        clone(TestBlockBuilder().build()));
  }
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given account, ametsuchi queries
 * @when valid block query is invalid (no can_get_blocks permission)
 * @then Query Processor should return an observable with blockError
 */
TEST_F(QueryProcessorTest, GetBlocksQueryNoPerms) {
  auto block_number = 5;
  auto block_query = getBlocksQuery(kAccountId);

  EXPECT_CALL(*wsv_queries, getSignatories(kAccountId))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*qry_exec, validate(_)).WillOnce(Return(false));

  auto wrapper =
      make_test_subscriber<CallExact>(qpi->blocksQueryHandle(block_query), 1);
  wrapper.subscribe([](auto response) {
    ASSERT_NO_THROW({
      boost::apply_visitor(framework::SpecifiedVisitor<
                               shared_model::interface::BlockErrorResponse>(),
                           response->get());
    });
  });
  for (int i = 0; i < block_number; i++) {
    storage->notifier.get_subscriber().on_next(
        clone(TestBlockBuilder()
                  .height(1)
                  .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
                  .build()));
  }
  ASSERT_TRUE(wrapper.validate());
}
