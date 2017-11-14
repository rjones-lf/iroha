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

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

#include "framework/test_subscriber.hpp"
#include "model/transaction_response.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

using namespace iroha;
using namespace iroha::network;
using namespace iroha::validation;
using namespace iroha::torii;
using namespace iroha::model;
using namespace iroha::ametsuchi;
using namespace framework::test_subscriber;

using ::testing::A;
using ::testing::Return;
using ::testing::_;

class TransactionProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    pcs = std::make_shared<MockPeerCommunicationService>();
    validation = std::make_shared<MockStatelessValidator>();
    mp = std::make_shared<MockMstProcessorDummy>();

    rxcpp::subjects::subject<Proposal> prop_notifier;
    rxcpp::subjects::subject<Commit> commit_notifier;
    rxcpp::subjects::subject<DataType> mst_notifier;

    EXPECT_CALL(*pcs, on_proposal())
        .WillRepeatedly(Return(prop_notifier.get_observable()));

    EXPECT_CALL(*pcs, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));

    EXPECT_CALL(*mp, onPreparedTransactionsImpl())
        .WillRepeatedly(Return(mst_notifier.get_observable()));
    EXPECT_CALL(*mp, onExpiredTransactionsImpl())
        .WillRepeatedly(Return(mst_notifier.get_observable()));

    tp = std::make_shared<TransactionProcessorImpl>(pcs, validation, mp);
  }

  std::shared_ptr<MockPeerCommunicationService> pcs;
  std::shared_ptr<MockStatelessValidator> validation;
  std::shared_ptr<TransactionProcessorImpl> tp;
  std::shared_ptr<MockMstProcessorDummy> mp;
};

/**
 * Transaction processor test case, when handling stateless valid transaction
 */
TEST_F(TransactionProcessorTest, ValidTransaction) {
  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(1);

  EXPECT_CALL(*validation, validate(A<const Transaction &>()))
      .WillRepeatedly(Return(true));

  auto tx = std::make_shared<Transaction>();
  tx->signatures.emplace_back();

  auto wrapper = make_test_subscriber<CallExact>(tp->transactionNotifier(), 1);
  wrapper.subscribe([](auto response) {
    auto resp = static_cast<TransactionResponse &>(*response);
    ASSERT_EQ(resp.current_status, TransactionResponse::STATELESS_VALIDATION_SUCCESS);
  });
  tp->transactionHandle(tx);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * Transaction processor test case, when handling invalid transaction
 */
TEST_F(TransactionProcessorTest, InvalidTransaction) {
  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(0);

  EXPECT_CALL(*validation, validate(A<const Transaction&>()))
  .WillRepeatedly(Return(false));

  auto tx = std::make_shared<Transaction>();
  tx->signatures.emplace_back();

  auto wrapper = make_test_subscriber<CallExact>(tp->transactionNotifier(), 1);
  wrapper.subscribe([](auto response) {
    auto resp = static_cast<TransactionResponse &>(*response);
    ASSERT_EQ(resp.current_status, TransactionResponse::STATELESS_VALIDATION_FAILED);
  });
  tp->transactionHandle(tx);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @when propagate tx with multiple signature
 * @then it goes to mst and doesn't go to PeerCommunicationService
 */
TEST_F(TransactionProcessorTest, MultisigTransaction) {
  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(1);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(0);

  EXPECT_CALL(*validation, validate(A<const Transaction &>()))
      .WillRepeatedly(Return(true));

  auto tx = std::make_shared<Transaction>();
  // ensure we have bigger quorum than signatures
  tx->quorum = 2;

  auto wrapper = make_test_subscriber<CallExact>(tp->transactionNotifier(), 1);
  wrapper.subscribe([](auto response) {
    auto resp = static_cast<TransactionResponse &>(*response);
    ASSERT_EQ(resp.current_status,
              TransactionResponse::STATELESS_VALIDATION_SUCCESS);
  });
  tp->transactionHandle(tx);

  ASSERT_TRUE(wrapper.validate());
}
