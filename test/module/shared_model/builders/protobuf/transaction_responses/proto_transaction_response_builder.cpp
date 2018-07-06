/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "framework/specified_visitor.hpp"

using shared_model::proto::TransactionStatusBuilder;

template <typename T>
class ProtoTransactionStatusBuilderTest : public ::testing::Test {};

template <typename Iface,
          TransactionStatusBuilder (TransactionStatusBuilder::*Member)(),
          iroha::protocol::TxStatus Status>
struct TransactionResponseBuilderTestCase {
  using IfaceType = Iface;
  static constexpr auto member = Member;
  static constexpr auto status = Status;
};

using TransactionResponsTypes =
    ::testing::Types<TransactionResponseBuilderTestCase<
                         shared_model::interface::StatelessFailedTxResponse,
                         &TransactionStatusBuilder::statelessValidationFailed,
                         iroha::protocol::STATELESS_VALIDATION_FAILED>,
                     TransactionResponseBuilderTestCase<
                         shared_model::interface::StatelessValidTxResponse,
                         &TransactionStatusBuilder::statelessValidationSuccess,
                         iroha::protocol::STATELESS_VALIDATION_SUCCESS>,
                     TransactionResponseBuilderTestCase<
                         shared_model::interface::StatefulFailedTxResponse,
                         &TransactionStatusBuilder::statefulValidationFailed,
                         iroha::protocol::STATEFUL_VALIDATION_FAILED>,
                     TransactionResponseBuilderTestCase<
                         shared_model::interface::StatefulValidTxResponse,
                         &TransactionStatusBuilder::statefulValidationSuccess,
                         iroha::protocol::STATEFUL_VALIDATION_SUCCESS>,
                     TransactionResponseBuilderTestCase<
                         shared_model::interface::CommittedTxResponse,
                         &TransactionStatusBuilder::committed,
                         iroha::protocol::COMMITTED>,
                     TransactionResponseBuilderTestCase<
                         shared_model::interface::NotReceivedTxResponse,
                         &TransactionStatusBuilder::notReceived,
                         iroha::protocol::NOT_RECEIVED> >;
TYPED_TEST_CASE(ProtoTransactionStatusBuilderTest, TransactionResponsTypes);

/**
 * @given expected transaction status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TYPED_TEST(ProtoTransactionStatusBuilderTest, TestStatusType) {
  using StatusType = typename TypeParam::IfaceType;

  auto expected_status = TypeParam::status;
  auto expected_hash = std::string(32, '1');

  auto response = (TransactionStatusBuilder().*TypeParam::member)()
                      .txHash(shared_model::crypto::Hash(expected_hash))
                      .build();

  ASSERT_NO_THROW(boost::apply_visitor(
      framework::SpecifiedVisitor<StatusType>(),
      response.get()));

  auto proto_status = response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given fields for transaction status object
 * @when TransactionStatusBuilder is invoked twice with the same configuration
 * @then Two constructed TransactionStatus objects are identical
 */
TEST(ProtoTransactionStatusBuilderTest, SeveralObjectsFromOneBuilder) {
  auto expected_status = iroha::protocol::NOT_RECEIVED;
  auto expected_hash = std::string(32, '1');

  auto state = TransactionStatusBuilder().notReceived().txHash(
      shared_model::crypto::Hash(expected_hash));

  auto response1 = state.build();
  auto response2 = state.build();

  ASSERT_EQ(response1, response2);
  ASSERT_EQ(response1.getTransport().tx_hash(), expected_hash);
  ASSERT_EQ(response1.getTransport().tx_status(), expected_status);
}
