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

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/query_responses/proto_error_query_response.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/keypair.hpp"
#include "execution/query_execution.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
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
    created_time = iroha::time::now();
    account_id = "account@domain";
    counter = 1048576;
  }

  auto getBlocksQuery(const std::string &creator_account_id) {
    return TestUnsignedBlocksQueryBuilder()
        .createdTime(created_time)
        .creatorAccountId(creator_account_id)
        .queryCounter(counter)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  decltype(iroha::time::now()) created_time;
  std::string account_id;
  uint64_t counter;
  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

  std::vector<shared_model::interface::types::PubkeyType> signatories = {
      keypair.publicKey()};
  shared_model::interface::RolePermissionSet perms;
  std::vector<std::string> roles;
};

/**
 * @given account, ametsuchi queries and query processing factory
 * @when stateless validation error
 * @then Query Processor should return ErrorQueryResponse
 */
TEST_F(QueryProcessorTest, QueryProcessorWhereInvokeInvalidQuery) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto storage = std::make_shared<MockStorage>();
  auto qpf =
      std::make_unique<QueryProcessingFactory>(wsv_queries, block_queries);

  iroha::torii::QueryProcessorImpl qpi(storage);

  auto query = TestUnsignedQueryBuilder()
                   .createdTime(created_time)
                   .creatorAccountId(account_id)
                   .getAccount(account_id)
                   .queryCounter(counter)
                   .build()
                   .signAndAddSignature(keypair)
                   .finish();

  std::shared_ptr<shared_model::interface::Account> shared_account = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());

  auto role = "admin";
  roles = {role};
  perms = {shared_model::interface::permissions::Role::kGetMyAccount};

  EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
  EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_queries));
  EXPECT_CALL(*wsv_queries, getAccount(account_id))
      .WillOnce(Return(shared_account));
  EXPECT_CALL(*wsv_queries, getAccountRoles(account_id))
      .Times(2)
      .WillRepeatedly(Return(roles));
  EXPECT_CALL(*wsv_queries, getRolePermissions(role)).WillOnce(Return(perms));
  EXPECT_CALL(*wsv_queries, getSignatories(account_id))
      .WillRepeatedly(Return(signatories));

  auto wrapper = make_test_subscriber<CallExact>(qpi.queryNotifier(), 1);
  wrapper.subscribe([](auto response) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::AccountResponse>(),
        response->get()));
  });
  qpi.queryHandle(
      std::make_shared<shared_model::proto::Query>(query.getTransport()));
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given account, ametsuchi queries and query processing factory
 * @when signed with wrong key
 * @then Query Processor should return StatefulFailed
 */
TEST_F(QueryProcessorTest, QueryProcessorWithWrongKey) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto storage = std::make_shared<MockStorage>();
  auto qpf =
      std::make_unique<QueryProcessingFactory>(wsv_queries, block_queries);

  iroha::torii::QueryProcessorImpl qpi(storage);

  auto query = TestUnsignedQueryBuilder()
                   .createdTime(created_time)
                   .creatorAccountId(account_id)
                   .getAccount(account_id)
                   .queryCounter(counter)
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair())
                   .finish();

  std::shared_ptr<shared_model::interface::Account> shared_account = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());
  auto role = "admin";
  roles = {role};
  perms = {shared_model::interface::permissions::Role::kGetMyAccount};

  EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
  EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_queries));
  EXPECT_CALL(*wsv_queries, getSignatories(account_id))
      .WillRepeatedly(Return(signatories));

  auto wrapper = make_test_subscriber<CallExact>(qpi.queryNotifier(), 1);
  wrapper.subscribe([](auto response) {
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::QueryErrorResponseChecker<
            shared_model::interface::StatefulFailedErrorResponse>(),
        response->get()));
  });
  qpi.queryHandle(
      std::make_shared<shared_model::proto::Query>(query.getTransport()));
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given account, ametsuchi queries and query processing factory
 * @when valid block query is send
 * @then Query Processor should start emitting BlockQueryRespones to the
 * observable
 */
TEST_F(QueryProcessorTest, GetBlocksQuery) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto storage = std::make_shared<MockStorage>();
  auto blockNumber = 5;
  auto qpf =
      std::make_unique<QueryProcessingFactory>(wsv_queries, block_queries);

  iroha::torii::QueryProcessorImpl qpi(storage);

  auto blockQuery = TestUnsignedBlocksQueryBuilder()
                        .createdTime(created_time)
                        .creatorAccountId(account_id)
                        .queryCounter(counter)
                        .build()
                        .signAndAddSignature(keypair)
                        .finish();

  std::shared_ptr<shared_model::interface::Account> shared_account = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());

  auto role = "admin";
  std::vector<std::string> roles = {role};
  perms = {shared_model::interface::permissions::Role::kGetMyAccount,
           shared_model::interface::permissions::Role::kGetBlocks};
  EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
  EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_queries));
  EXPECT_CALL(*wsv_queries, getAccountRoles(account_id))
      .WillOnce(Return(roles));
  EXPECT_CALL(*wsv_queries, getRolePermissions(role)).WillOnce(Return(perms));
  EXPECT_CALL(*wsv_queries, getSignatories(account_id))
      .WillRepeatedly(Return(signatories));

  auto wrapper = make_test_subscriber<CallExact>(
      qpi.blocksQueryHandle(std::make_shared<shared_model::proto::BlocksQuery>(
          blockQuery.getTransport())),
      blockNumber);
  wrapper.subscribe([](auto response) {
    ASSERT_NO_THROW({
      boost::apply_visitor(
          framework::SpecifiedVisitor<shared_model::interface::BlockResponse>(),
          response->get());
    });
  });
  for (int i = 0; i < blockNumber; i++) {
    storage->notifier.get_subscriber().on_next(
        clone(TestBlockBuilder()
                  .height(1)
                  .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
                  .build()));
  }
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given account, ametsuchi queries and query processing factory
 * @when valid block query is invalid (no can_get_blocks permission)
 * @then Query Processor should return an observable with blockError
 */
TEST_F(QueryProcessorTest, GetBlocksQueryNoPerms) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto storage = std::make_shared<MockStorage>();
  auto blockNumber = 5;
  auto qpf =
      std::make_unique<QueryProcessingFactory>(wsv_queries, block_queries);

  iroha::torii::QueryProcessorImpl qpi(storage);

  auto blockQuery = TestUnsignedBlocksQueryBuilder()
                        .createdTime(created_time)
                        .creatorAccountId(account_id)
                        .queryCounter(counter)
                        .build()
                        .signAndAddSignature(keypair)
                        .finish();

  std::shared_ptr<shared_model::interface::Account> shared_account = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());

  auto role = "admin";
  std::vector<std::string> roles = {role};
  perms = {shared_model::interface::permissions::Role::kGetMyAccount};
  EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
  EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_queries));
  EXPECT_CALL(*wsv_queries, getAccountRoles(account_id))
      .WillOnce(Return(roles));
  EXPECT_CALL(*wsv_queries, getRolePermissions(role)).WillOnce(Return(perms));
  EXPECT_CALL(*wsv_queries, getSignatories(account_id))
      .WillRepeatedly(Return(signatories));

  auto wrapper = make_test_subscriber<CallExact>(
      qpi.blocksQueryHandle(std::make_shared<shared_model::proto::BlocksQuery>(
          blockQuery.getTransport())),
      1);
  wrapper.subscribe([](auto response) {
    ASSERT_NO_THROW({
      boost::apply_visitor(framework::SpecifiedVisitor<
                               shared_model::interface::BlockErrorResponse>(),
                           response->get());
    });
  });
  for (int i = 0; i < blockNumber; i++) {
    storage->notifier.get_subscriber().on_next(
        clone(TestBlockBuilder()
                  .height(1)
                  .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
                  .build()));
  }
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given admin with permisisons to get blocks
 * @and user with no permissions to get blocks
 * @when admin sends blocks query
 * @and user sends blocks query
 * @then admin will get only block response
 * @and user will get only block error response with stateful invalid message
 */
TEST_F(QueryProcessorTest, NoOneSeesStatefulInvalidButCaller) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto storage = std::make_shared<MockStorage>();

  auto qpf =
      std::make_unique<QueryProcessingFactory>(wsv_queries, block_queries);

  iroha::torii::QueryProcessorImpl qpi(storage);

  std::shared_ptr<shared_model::interface::Account> account_with_perms = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());

  std::shared_ptr<shared_model::interface::Account> account_without_perms =
      clone(
          shared_model::proto::AccountBuilder().accountId(account_id).build());

  auto admin_account_id = "admin@test";
  auto user_account_id = "user@test";

  auto admin_role = "admin";
  auto user_role = "user";

  shared_model::interface::RolePermissionSet admin_perms = {
      shared_model::interface::permissions::Role::kGetMyAccount,
      shared_model::interface::permissions::Role::kGetBlocks};
  shared_model::interface::RolePermissionSet user_perms = {
      shared_model::interface::permissions::Role::kGetMyAccount};

  EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_queries));
  EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_queries));

  EXPECT_CALL(*wsv_queries, getSignatories(admin_account_id))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*wsv_queries, getSignatories(user_account_id))
      .WillRepeatedly(Return(signatories));

  EXPECT_CALL(*wsv_queries, getAccountRoles(admin_account_id))
      .WillOnce(Return(std::vector<std::string>{admin_role}));
  EXPECT_CALL(*wsv_queries, getAccountRoles(user_account_id))
      .WillOnce(Return(std::vector<std::string>{user_role}));

  EXPECT_CALL(*wsv_queries, getRolePermissions(admin_role))
      .WillOnce(Return(admin_perms));
  EXPECT_CALL(*wsv_queries, getRolePermissions(user_role))
      .WillOnce(Return(user_perms));

  auto expected_block =
      TestBlockBuilder()
          .height(1)
          .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
          .build();

  auto admin_block_query = getBlocksQuery(admin_account_id);

  // check that admin can get block and do not get anything but block responses
  auto admin_wrapper = make_test_subscriber<CallExact>(
      qpi.blocksQueryHandle(std::make_shared<shared_model::proto::BlocksQuery>(
          admin_block_query.getTransport())),
      1);
  admin_wrapper.subscribe([&expected_block](
                              const std::shared_ptr<
                                  shared_model::interface::BlockQueryResponse>
                                  &block) {
    ASSERT_NO_THROW({
      auto &block_response = boost::apply_visitor(
          framework::SpecifiedVisitor<shared_model::interface::BlockResponse>(),
          block->get());
      ASSERT_EQ(block_response.block(), expected_block);
    });
  });

  auto user_block_query = getBlocksQuery(user_account_id);

  // check that user without can_get_blocks permission will not get anything but
  // block error response
  auto user_wrapper = make_test_subscriber<CallExact>(
      qpi.blocksQueryHandle(std::make_shared<shared_model::proto::BlocksQuery>(
          user_block_query.getTransport())),
      1);
  user_wrapper.subscribe(
      [](const std::shared_ptr<shared_model::interface::BlockQueryResponse>
             &block) {
        ASSERT_NO_THROW({
          auto &block_response = boost::apply_visitor(
              framework::SpecifiedVisitor<
                  shared_model::interface::BlockErrorResponse>(),
              block->get());
          ASSERT_EQ(block_response.message(), "Stateful invalid");
        });
      });

  // apply expected block to the ledger
  storage->notifier.get_subscriber().on_next(clone(expected_block));

  ASSERT_TRUE(admin_wrapper.validate());
  ASSERT_TRUE(user_wrapper.validate());
}
