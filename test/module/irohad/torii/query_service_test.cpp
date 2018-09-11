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
#include "module/shared_model/builders/protobuf/common_objects/proto_account_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_response_builder.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace torii;

using namespace iroha;
using namespace iroha::torii;

using namespace shared_model::detail;
using namespace shared_model::interface;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
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
                    generateKeypair())
            .finish());
  }

  void init() {
    query_service = std::make_shared<QueryService>(query_processor);
  }

  std::unique_ptr<shared_model::proto::QueryResponse> getResponse() {
    static auto account = shared_model::proto::AccountBuilder()
                              .accountId("a")
                              .domainId("ru")
                              .quorum(2)
                              .build();
    return clone(TestQueryResponseBuilder()
                     .accountResponse(account, {"user"})
                     .queryHash(query->hash())
                     .build());
  }

  std::shared_ptr<shared_model::proto::Query> query;
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
  EXPECT_CALL(*query_processor,
              queryHandle(
                  // match by shared_ptr's content
                  Truly([this](const shared_model::interface::Query &rhs) {
                    return rhs == *query;
                  })))
      .WillOnce(Invoke([this](auto &) { return this->getResponse(); }));
  init();

  protocol::QueryResponse response;
  query_service->Find(query->getTransport(), response);
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_EQ(resp, *getResponse());
}

/**
 * @given query
 * @when query is sent to query service twice
 * @then query processor will be invoked once and second response will have
 * STATELESS_INVALID status
 */
TEST_F(QueryServiceTest, InvalidWhenDuplicateHash) {
  // two same queries => only first query handled by query processor
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Invoke([this](auto &) {
    return this->getResponse();
  }));

  init();

  protocol::QueryResponse response;
  query_service->Find(query->getTransport(), response);

  // second call of the same query
  query_service->Find(query->getTransport(), response);
  ASSERT_TRUE(response.has_error_response());
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatelessFailedErrorResponse>(),
      resp.get()));
}
