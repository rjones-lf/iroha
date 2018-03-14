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
#include "module/shared_model/builders/transaction_responses/transaction_builders_common.hpp"

using shared_model::proto::TransactionStatusBuilder;

/**
 * @given expected transaction stateless invalid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestStatelessFailedStatus) {
  using StatelessFailedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatelessFailedTxResponse>;

  auto expected_status = iroha::protocol::STATELESS_VALIDATION_FAILED;
  auto expected_hash = std::string(32, '1');

  auto stateless_invalid_response =
      TransactionStatusBuilder()
          .statelessValidationFailed()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<StatelessFailedStatusType>(),
      stateless_invalid_response.get());

  auto proto_status = stateless_invalid_response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given expected transaction stateless valid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestStatelessValidStatus) {
  using StatelessValidStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatelessValidTxResponse>;

  auto expected_status = iroha::protocol::STATELESS_VALIDATION_SUCCESS;
  auto expected_hash = std::string(32, '1');

  auto stateless_invalid_response =
      TransactionStatusBuilder()
          .statelessValidationSuccess()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<StatelessValidStatusType>(),
      stateless_invalid_response.get());

  auto proto_status = stateless_invalid_response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given expected transaction stateful invalid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestStatefulFailedStatus) {
  using StatefulFailedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulFailedTxResponse>;

  auto expected_status = iroha::protocol::STATEFUL_VALIDATION_FAILED;
  auto expected_hash = std::string(32, '1');

  auto stateful_invalid_response =
      TransactionStatusBuilder()
          .statefulValidationFailed()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<StatefulFailedStatusType>(),
      stateful_invalid_response.get());

  auto proto_status = stateful_invalid_response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given expected transaction stateful valid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestStatefulValidStatus) {
  using StatefulValidStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulValidTxResponse>;

  auto expected_status = iroha::protocol::STATEFUL_VALIDATION_SUCCESS;
  auto expected_hash = std::string(32, '1');

  auto stateful_invalid_response =
      TransactionStatusBuilder()
          .statefulValidationSuccess()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<StatefulValidStatusType>(),
      stateful_invalid_response.get());

  auto proto_status = stateful_invalid_response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given expected transaction commit status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestCommittedStatus) {
  using CommittedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::CommittedTxResponse>;

  auto expected_status = iroha::protocol::COMMITTED;
  auto expected_hash = std::string(32, '1');

  auto committed_response =
      TransactionStatusBuilder()
          .committed()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<CommittedStatusType>(),
      committed_response.get());

  auto proto_status = committed_response.getTransport();
  ASSERT_EQ(proto_status.tx_status(), expected_status);
  ASSERT_EQ(proto_status.tx_hash(), expected_hash);
}

/**
 * @given expected transaction not received status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestNotReceivedStatus) {
  using NotReceivedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::NotReceivedTxResponse>;

  auto expected_status = iroha::protocol::NOT_RECEIVED;
  auto expected_hash = std::string(32, '1');

  auto not_received_response =
      TransactionStatusBuilder()
          .notReceived()
          .txHash(shared_model::crypto::Hash(expected_hash))
          .build();

  boost::apply_visitor(
      verifyType<NotReceivedStatusType>(),
      not_received_response.get());

  auto proto_status = not_received_response.getTransport();
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
