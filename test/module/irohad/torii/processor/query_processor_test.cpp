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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "model/query_execution.hpp"
#include "model/query_responses/stateless_response.hpp"
#include "network/ordering_gate.hpp"
#include "torii/processor/query_processor_impl.hpp"

using namespace iroha;
using ::testing::Return;
using ::testing::_;
using ::testing::A;

/**
 * Mock for stateless validation
 */
class StatelessValidationMock : public validation::StatelessValidator {
 public:
  MOCK_CONST_METHOD1(validate, bool(const model::Transaction &transaction));
  MOCK_CONST_METHOD1(validate, bool(const model::Query &query));
};

/**
 * Mock for wsv query
 */
class WsvQueryMock : public ametsuchi::WsvQuery {
 public:
  MOCK_METHOD1(getAccount,
               nonstd::optional<model::Account>(const std::string &account_id));
  MOCK_METHOD1(getSignatories, nonstd::optional<std::vector<ed25519::pubkey_t>>(
                                   const std::string &account_id));
  MOCK_METHOD1(getAsset,
               nonstd::optional<model::Asset>(const std::string &asset_id));
  MOCK_METHOD2(getAccountAsset,
               nonstd::optional<model::AccountAsset>(
                   const std::string &account_id, const std::string &asset_id));
  MOCK_METHOD0(getPeers, nonstd::optional<std::vector<model::Peer>>());
};

/**
 * Mock for block query
 */
class BlockQueryMock : public ametsuchi::BlockQuery {
  MOCK_METHOD1(getAccountTransactions,
               rxcpp::observable<model::Transaction>(std::string account_id));
  MOCK_METHOD2(getBlocks,
               rxcpp::observable<model::Block>(uint32_t from, uint32_t to));
};

/**
 * Mock for query processing factory
 */
class QpfMock : public model::QueryProcessingFactory {
 public:
  MOCK_METHOD1(execute, std::shared_ptr<iroha::model::QueryResponse>(
                            const model::Query &query));
};

TEST(QueryProcessorTest, QueryProcessorWhereInvokeValidQuery) {
  WsvQueryMock wsv_query;
  BlockQueryMock block_query;
  model::QueryProcessingFactory qpf(wsv_query, block_query);

  StatelessValidationMock validation;
  EXPECT_CALL(validation, validate(A<const model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::torii::QueryProcessorImpl qpi(qpf, validation);
  model::Query query;
  qpi.query_notifier()
      .filter([](auto response) {
        return instanceof <model::QueryStatelessResponse>(response);
      })
      .subscribe([](auto response) {
        auto resp = static_cast<model::QueryStatelessResponse &>(*response);
        ASSERT_EQ(resp.passed, true);
      });
  qpi.query_handle(query);
}

TEST(QueryProcessorTest, QueryProcessorWhereInvokeInValidQuery) {
  WsvQueryMock wsv_query;
  BlockQueryMock block_query;
  model::QueryProcessingFactory qpf(wsv_query, block_query);

  StatelessValidationMock validation;
  EXPECT_CALL(validation, validate(A<const model::Query &>()))
      .WillRepeatedly(Return(false));

  iroha::torii::QueryProcessorImpl qpi(qpf, validation);
  model::Query query;
  qpi.query_notifier()
      .filter([](auto response) {
        return instanceof <model::QueryStatelessResponse>(response);
      })
      .subscribe([](auto response) {
        auto resp = static_cast<model::QueryStatelessResponse &>(*response);
        ASSERT_EQ(resp.passed, false);
      });
  qpi.query_handle(query);
}
