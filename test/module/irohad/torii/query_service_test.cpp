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

#include "torii/query_service.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "builders/protobuf/queries.hpp"
#include "module/irohad/torii/torii_mocks.hpp"

using namespace torii;

using namespace iroha;
using namespace iroha::torii;

using namespace shared_model::detail;
using namespace shared_model::interface;
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::Truly;

class QueryServiceTest : public ::testing::Test {
 public:
  void SetUp() override {
    query_processor = std::make_shared<MockQueryProcessor>();
    // any query
    query = std::make_shared<shared_model::proto::Query>(
        shared_model::proto::QueryBuilder()
            .creatorAccountId("user@domain")
            .createdTime(iroha::time::now())
            .queryCounter(1)
            .getAccount("user@domain")
            .build()
            .signAndAddSignature(
                shared_model::crypto::DefaultCryptoAlgorithmType::
                    generateKeypair()));

    // TODO: IR-1041 Update to query response builders (kamilsa, 04.03.2018)
    protocol::QueryResponse response;
    response.set_query_hash(
        shared_model::crypto::toBinaryString(query->hash()));
    auto account_response = response.mutable_account_response();
    account_response->add_account_roles("user");
    auto account = account_response->mutable_account();
    account->set_domain_id("ru");
    account->set_account_id("a");
    account->set_quorum(2);

    model_response = std::make_shared<shared_model::proto::QueryResponse>(
        std::move(response));
  }

  void init() {
    query_service = std::make_shared<QueryService>(query_processor);
  }

  std::shared_ptr<shared_model::proto::Query> query;
  std::shared_ptr<shared_model::proto::QueryResponse> model_response;
  std::shared_ptr<QueryService> query_service;
  std::shared_ptr<MockQueryProcessor> query_processor;
};

/**
 * @given query and expected valid response
 * @when query is sent to query service and query_processor processes query
 * @then expected response is returned
 */
TEST_F(QueryServiceTest, ValidWhenUniqueHash) {
  // unique query => query handled by query processor
  rxcpp::subjects::subject<
      std::shared_ptr<shared_model::interface::QueryResponse>>
      notifier;
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(Return(notifier.get_observable()));
  EXPECT_CALL(
      *query_processor,
      queryHandle(
          // match by shared_ptr's content
          Truly([this](std::shared_ptr<shared_model::interface::Query> rhs) {
            return *rhs == *query;
          })))
      .WillOnce(Invoke([this, &notifier](auto q) {
        notifier.get_subscriber().on_next(model_response);
      }));
  init();

  protocol::QueryResponse response;
  query_service->Find(query->getTransport(), response);
  ASSERT_EQ(response.SerializeAsString(),
            model_response->getTransport().SerializeAsString());
}

/**
 * @given query and expected response
 * @when query is sent to query service and query_processor does not process
 * query
 * @then NOT_SUPPORTED error response is returned
 */
TEST_F(QueryServiceTest, InvalidWhenUniqueHash) {
  // unique query => query handled by query processor
  rxcpp::subjects::subject<
      std::shared_ptr<shared_model::interface::QueryResponse>>
      notifier;
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(Return(notifier.get_observable()));
  EXPECT_CALL(
      *query_processor,
      queryHandle(
          // match by shared_ptr's content
          Truly([this](std::shared_ptr<shared_model::interface::Query> rhs) {
            return *rhs == *query;
          })))
      .WillOnce(Return());
  init();

  protocol::QueryResponse response;
  query_service->Find(query->getTransport(), response);
  ASSERT_TRUE(response.has_error_response());
  ASSERT_EQ(response.error_response().reason(),
            protocol::ErrorResponse::NOT_SUPPORTED);
}

/**
 * @given query
 * @when query is sent to query service twice
 * @then query processor will be invoked once and second response will have
 * STATELESS_INVALID status
 */
TEST_F(QueryServiceTest, InvalidWhenDuplicateHash) {
  // two same queries => only first query handled by query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(
          Return(rxcpp::observable<>::empty<std::shared_ptr<QueryResponse>>()));
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Return());

  init();

  protocol::QueryResponse response;
  query_service->Find(query->getTransport(), response);

  // second call of the same query
  query_service->Find(query->getTransport(), response);
  ASSERT_TRUE(response.has_error_response());
  ASSERT_EQ(response.error_response().reason(),
            protocol::ErrorResponse::STATELESS_INVALID);
}
