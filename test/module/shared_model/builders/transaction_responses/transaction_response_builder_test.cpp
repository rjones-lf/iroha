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
#include "builders/default_builders.hpp"
#include "builders/transaction_responses/transaction_status_builder.hpp"
#include "module/shared_model/builders/transaction_responses/transaction_builders_common.hpp"

using shared_model::builder::TransactionStatusBuilder;

/**
 * @given expected transaction stateless invalid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, StatelessFailedStatus) {
  using StatelessFailedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatelessFailedTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto stateless_invalid_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .statelessValidationFailed()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<StatelessFailedStatusType>(),
                       stateless_invalid_response->get());

  ASSERT_EQ(stateless_invalid_response->transactionHash(), expected_hash);
}

/**
 * @given expected transaction stateless valid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, StatelessValidStatus) {
  using StatelessValidStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatelessValidTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto stateless_valid_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .statelessValidationSuccess()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<StatelessValidStatusType>(),
                       stateless_valid_response->get());

  ASSERT_EQ(stateless_valid_response->transactionHash(), expected_hash);
}

/**
 * @given expected transaction stateful invalid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, StatefulFailedStatus) {
  using StatefulFailedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulFailedTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto stateful_invalid_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .statefulValidationFailed()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<StatefulFailedStatusType>(),
                       stateful_invalid_response->get());

  ASSERT_EQ(stateful_invalid_response->transactionHash(), expected_hash);
}

/**
 * @given expected transaction stateful valid status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, StatefulValidStatus) {
  using StatefulValidStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulValidTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto stateful_valid_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .statefulValidationSuccess()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<StatefulValidStatusType>(),
                       stateful_valid_response->get());

  ASSERT_EQ(stateful_valid_response->transactionHash(), expected_hash);
}

/**
 * @given expected transaction committed status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, CommittedStatus) {
  using CommittedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::CommittedTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto committed_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .committed()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<CommittedStatusType>(),
                       committed_response->get());

  ASSERT_EQ(committed_response->transactionHash(), expected_hash);
}

/**
 * @given expected transaction not received status and hash
 * @when model object is built using these status and hash
 * @then built object has expected status and hash
 */
TEST(TransactionResponseBuilderTest, NotReceivedStatus) {
  using NotReceivedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::NotReceivedTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto not_received_response =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .notReceived()
          .txHash(expected_hash)
          .build();

  // check if type in reponse is as expected
  boost::apply_visitor(verifyType<NotReceivedStatusType>(),
                       not_received_response->get());

  ASSERT_EQ(not_received_response->transactionHash(), expected_hash);
}

/**
 * @given fields for transaction status object
 * @when TransactionStatusBuilder is invoked twice with the same configuration
 * @then Two constructed TransactionStatus objects are identical
 */
TEST(ProtoTransactionStatusBuilderTest, SeveralObjectsFromOneBuilder) {
  using NotReceivedStatusType = shared_model::detail::PolymorphicWrapper<
      shared_model::interface::NotReceivedTxResponse>;

  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));

  auto state =
      TransactionStatusBuilder<shared_model::proto::TransactionStatusBuilder>()
          .notReceived()
          .txHash(expected_hash);

  auto response1 = state.build();
  auto response2 = state.build();

  ASSERT_EQ(*response1, *response2);
  ASSERT_EQ(response1->transactionHash(), expected_hash);

  boost::apply_visitor(verifyType<NotReceivedStatusType>(), response1->get());
}
