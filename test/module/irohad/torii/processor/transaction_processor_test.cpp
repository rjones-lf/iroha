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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "torii/processor/transaction_processor_impl.hpp"
#include "model/tx_responses/stateless_response.hpp"
#include "network/ordering_gate.hpp"

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
 * Mock for peer communication service
 */
class PcsMock : public network::PeerCommunicationService {
 public:
  MOCK_METHOD1(propagate_transaction, void(model::Transaction transaction));

  MOCK_METHOD0(on_proposal, rxcpp::observable<model::Proposal>());

  MOCK_METHOD0(on_commit,
               rxcpp::observable < rxcpp::observable < model::Block >> ());
};

/**
 * Mock for ordering service
 */
class OsMock : public network::OrderingGate {
 public:
  MOCK_METHOD1(propagate_transaction, void(
      const model::Transaction &transaction));

  MOCK_METHOD0(on_proposal, rxcpp::observable<model::Proposal>());
};

/**
 * Transaction processor test case, when handling stateless valid transaction
 */
TEST(TransactionProcessorTest,
     TransactionProcessorWhereInvokeValidTransaction) {

  PcsMock pcs;
  EXPECT_CALL(pcs, propagate_transaction(_)).Times(1);

  StatelessValidationMock validation;
  EXPECT_CALL(validation, validate(A<const model::Transaction&>())).WillRepeatedly(Return(true));

  iroha::torii::TransactionProcessorImpl tp(pcs, validation);
  model::Transaction tx;
  // TODO subscribe with testable subscriber
  tp.transaction_notifier().subscribe([](auto response) {
    auto resp = static_cast<model::TransactionStatelessResponse &>(*response);
    ASSERT_EQ(resp.passed, true);
  });
  tp.transaction_handle(tx);
}

/**
 * Transaction processor test case, when handling invalid transaction
 */
TEST(TransactionProcessorTest,
     TransactionProcessorWhereInvokeInvalidTransaction) {

  PcsMock pcs;
  EXPECT_CALL(pcs, propagate_transaction(_)).Times(0);

  StatelessValidationMock validation;
  EXPECT_CALL(validation, validate(A<const model::Transaction&>())).WillRepeatedly(Return(false));
  EXPECT_CALL(validation, validate(A<const model::Query&>())).WillRepeatedly(Return(false));

  iroha::torii::TransactionProcessorImpl tp(pcs, validation);
  model::Transaction tx;
  // TODO subscribe with testable subscriber
  tp.transaction_notifier().subscribe([](auto response) {
    auto resp = static_cast<model::TransactionStatelessResponse &>(*response);
    ASSERT_EQ(resp.passed, false);
  });
  tp.transaction_handle(tx);
}
