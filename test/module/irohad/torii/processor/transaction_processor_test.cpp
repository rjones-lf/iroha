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
#include "module/irohad/validation/validation_mocks.hpp"

#include "builders/protobuf/transaction.hpp"
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

using ::testing::_;
using ::testing::A;
using ::testing::Return;

class TransactionProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    pcs = std::make_shared<MockPeerCommunicationService>();

    rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>> prop_notifier;
    rxcpp::subjects::subject<Commit> commit_notifier;

    EXPECT_CALL(*pcs, on_proposal())
        .WillRepeatedly(Return(prop_notifier.get_observable()));

    EXPECT_CALL(*pcs, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));

    tp = std::make_shared<TransactionProcessorImpl>(pcs);
  }

  std::shared_ptr<MockPeerCommunicationService> pcs;
  std::shared_ptr<TransactionProcessorImpl> tp;
};

/**
 * Transaction processor test case, when handling stateless valid transaction
 */
TEST_F(TransactionProcessorTest,
       TransactionProcessorWhereInvokeValidTransaction) {
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(1);

  auto tx = shared_model::proto::TransactionBuilder()
                .creatorAccountId("user@domain")
                .createdTime(iroha::time::now())
                .txCounter(1)
                .setAccountQuorum("user@domain", 2)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair());

  auto wrapper = make_test_subscriber<CallExact>(tp->transactionNotifier(), 1);
  wrapper.subscribe([](auto response) {
    auto resp = static_cast<TransactionResponse &>(*response);
    ASSERT_EQ(resp.current_status,
              iroha::model::TransactionResponse::STATELESS_VALIDATION_SUCCESS);
  });
  tp->transactionHandle(std::shared_ptr<model::Transaction>(tx.makeOldModel()));

  ASSERT_TRUE(wrapper.validate());
}
