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

#include "model/permissions.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

#include "framework/test_subscriber.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/query_execution.hpp"
#include "network/ordering_gate.hpp"
#include "torii/processor/query_processor_impl.hpp"

#include "backend/protobuf/query_responses/proto_error_query_response.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/keypair.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::validation;
using namespace framework::test_subscriber;

using ::testing::A;
using ::testing::Return;
using ::testing::_;

class QueryProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    created_time = iroha::time::now();
    account_id = "account@domain";
    counter = 1048576;
  }

  decltype(iroha::time::now()) created_time;
  std::string account_id;
  uint64_t counter;
  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given account, ametsuchi queries and query processing factory
 * @when stateless validation error
 * @then Query Processor should return ErrorQueryResponse
 */
TEST_F(QueryProcessorTest, QueryProcessorWhereInvokeInvalidQuery) {
  auto wsv_queries = std::make_shared<MockWsvQuery>();
  auto block_queries = std::make_shared<MockBlockQuery>();
  auto qpf = std::make_unique<model::QueryProcessingFactory>(wsv_queries,
                                                             block_queries);

  iroha::torii::QueryProcessorImpl qpi(std::move(qpf));

  auto query = TestUnsignedQueryBuilder()
                   .createdTime(created_time)
                   .creatorAccountId(account_id)
                   .getAccount(account_id)
                   .queryCounter(counter)
                   .build()
                   .signAndAddSignature(keypair);

  auto qry_resp = std::make_shared<model::AccountResponse>();
  auto account = model::Account();
  account.account_id = account_id;
  qry_resp->account = account;
  std::shared_ptr<shared_model::interface::Account> shared_account = clone(
      shared_model::proto::AccountBuilder().accountId(account_id).build());
  auto role = "admin";
  std::vector<std::string> roles = {role};
  std::vector<std::string> perms = {iroha::model::can_get_my_account};

  EXPECT_CALL(*wsv_queries, getAccount(account_id))
      .WillOnce(Return(shared_account));
  EXPECT_CALL(*wsv_queries, getAccountRoles(account_id))
      .Times(2)
      .WillRepeatedly(Return(roles));
  EXPECT_CALL(*wsv_queries, getRolePermissions(role)).WillOnce(Return(perms));

  auto wrapper = make_test_subscriber<CallExact>(qpi.queryNotifier(), 1);
  wrapper.subscribe([](auto response) {
    auto resp = response->get();
    /// check if obtained response is error response
    boost::apply_visitor(
        [](auto val) {
          if (std::is_same<
                  decltype(val),
                  shared_model::detail::PolymorphicWrapper<
                      shared_model::interface::AccountResponse>>::value) {
            SUCCEED();
          } else {
            FAIL();
          }

        },
        resp);
  });
  qpi.queryHandle(
      std::make_shared<shared_model::proto::Query>(query.getTransport()));
}
