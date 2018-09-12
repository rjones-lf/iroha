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
#include "framework/specified_visitor.hpp"

#include "backend/protobuf/proto_tx_status_factory.hpp"

/**
 * @given status and hash
 * @when  model object is built using these status and hash, but with committed
 *        status
 * @then  built object has expected status and hash
 */
TEST(ProtoTransactionStatusBuilderTest, TestStatusType) {
  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));
  auto error_massage = std::string("error");

  auto response =
      shared_model::proto::ProtoTxStatusFactory().makeTxStatusCommitted(
          expected_hash, error_massage);

  ASSERT_EQ(response->transactionHash(), expected_hash);
  ASSERT_EQ(response->errorMessage(), error_massage);

  ASSERT_NO_THROW(
          boost::apply_visitor(framework::SpecifiedVisitor<
                                       shared_model::interface::CommittedTxResponse>(),
                               response->get()));
}

/**
 * @given fields for transaction status object
 * @when TransactionStatusBuilder is invoked twice with the same configuration
 * @then Two constructed TransactionStatus objects are identical
 */
TEST(ProtoTransactionStatusBuilderTest, SeveralObjectsFromOneBuilder) {
  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));
  auto error_massage = std::string("error");

  auto response1 =
      shared_model::proto::ProtoTxStatusFactory().makeTxStatusMstExpired(
          expected_hash, error_massage);

  auto response2 =
      shared_model::proto::ProtoTxStatusFactory().makeTxStatusMstExpired(
          expected_hash, error_massage);

  ASSERT_EQ(*response1, *response2);
}
