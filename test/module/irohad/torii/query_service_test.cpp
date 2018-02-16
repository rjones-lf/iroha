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
#include "generator/generator.hpp"
#include "module/irohad/torii/torii_mocks.hpp"

using namespace torii;

using namespace iroha;
using namespace iroha::torii;
using namespace iroha::model;
using namespace iroha::model::converters;

using ::testing::_;
using ::testing::Return;

class QueryServiceTest : public ::testing::Test {
 public:
  void SetUp() override {
    query_processor = std::make_shared<MockQueryProcessor>();
    query_factory = std::make_shared<PbQueryFactory>();
    query_response_factory = std::make_shared<PbQueryResponseFactory>();
    // any query
    query.mutable_payload()->mutable_get_account();

    // just random hex strings (same seed every time is ok here)
    query.mutable_signature()->set_pubkey(
        generator::random_blob<16>(0).to_hexstring());
    query.mutable_signature()->set_signature(
        generator::random_blob<32>(0).to_hexstring());
  }

  void init() {
    query_service = std::make_shared<QueryService>(
        query_factory, query_response_factory, query_processor);
  }

  protocol::Query query;
  protocol::QueryResponse response;
  std::shared_ptr<QueryService> query_service;
  std::shared_ptr<MockQueryProcessor> query_processor;
  std::shared_ptr<PbQueryFactory> query_factory;
  std::shared_ptr<PbQueryResponseFactory> query_response_factory;
};

TEST_F(QueryServiceTest, SubscribeQueryProcessorWhenInit) {
  // query service is subscribed to query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(Return(
          rxcpp::observable<>::empty<std::shared_ptr<model::QueryResponse>>()));

  init();
}

TEST_F(QueryServiceTest, ValidWhenUniqueHash) {
  // unique query => query handled by query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(Return(
          rxcpp::observable<>::empty<std::shared_ptr<model::QueryResponse>>()));
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Return());

  init();

  query_service->Find(query, response);
}

TEST_F(QueryServiceTest, InvalidWhenDuplicateHash) {
  // two same queries => only first query handled by query processor
  EXPECT_CALL(*query_processor, queryNotifier())
      .WillOnce(Return(
          rxcpp::observable<>::empty<std::shared_ptr<model::QueryResponse>>()));
  EXPECT_CALL(*query_processor, queryHandle(_)).WillOnce(Return());

  init();
  query_service->Find(query, response);
  query_service->Find(query, response);
}
