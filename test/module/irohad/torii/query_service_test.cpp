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
#include "builders/protobuf/queries.hpp"
#include "generator/generator.hpp"
#include "module/irohad/torii/torii_mocks.hpp"

using namespace torii;

using namespace iroha;
using namespace iroha::torii;

using namespace shared_model::detail;
using namespace shared_model::interface;
using ::testing::_;
using ::testing::Return;

class QueryServiceTest : public ::testing::Test {
 public:
  void SetUp() override {
    query_processor = std::make_shared<MockQueryProcessor>();
    // any query
    query = shared_model::proto::QueryBuilder()
                .creatorAccountId("user@domain")
                .createdTime(iroha::time::now())
                .queryCounter(1)
                .getAccountTransactions("user@domain")
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .getTransport();
  }

  void init() {
    query_service = std::make_shared<QueryService>(query_processor);
  }

  protocol::Query query;
  protocol::QueryResponse response;
  std::shared_ptr<QueryService> query_service;
  std::shared_ptr<MockQueryProcessor> query_processor;
};

TEST_F(QueryServiceTest, SubscribeQueryProcessorWhenInit) {
  // query service is subscribed to query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(
          Return(rxcpp::observable<>::empty<std::shared_ptr<QueryResponse>>()));

  init();
}

TEST_F(QueryServiceTest, ValidWhenUniqueHash) {
  // unique query => query handled by query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(
          Return(rxcpp::observable<>::empty<std::shared_ptr<QueryResponse>>()));
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Return());
  init();

  query_service->Find(query, response);
}

TEST_F(QueryServiceTest, InvalidWhenDuplicateHash) {
  // two same queries => only first query handled by query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(
          Return(rxcpp::observable<>::empty<std::shared_ptr<QueryResponse>>()));
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Return());

  init();
  query_service->Find(query, response);
  query_service->Find(query, response);
}
