/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_tx_status_factory.hpp"

#include <gtest/gtest.h>

#include "framework/specified_visitor.hpp"

/**
 * @given status and hash
 * @when  model object is built using these status and hash, but with committed
 *        status
 * @then  built object has expected status and hash
 */
TEST(ProtoTransactionStatusFactoryTest, TestStatusType) {
  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));
  auto error_massage = std::string("error");

  auto response = shared_model::proto::ProtoTxStatusFactory().makeCommitted(
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
 * @when  protoTransactionStatusFactory is invoked twice with the same
 *        configuration
 * @then  two constructed TransactionStatus objects are identical
 */
TEST(ProtoTransactionStatusFactoryTest, SeveralObjectsFromOneBuilder) {
  auto expected_hash = shared_model::crypto::Hash(std::string(32, '1'));
  auto error_massage = std::string("error");

  auto response1 = shared_model::proto::ProtoTxStatusFactory().makeMstExpired(
      expected_hash, error_massage);

  auto response2 = shared_model::proto::ProtoTxStatusFactory().makeMstExpired(
      expected_hash, error_massage);

  ASSERT_EQ(*response1, *response2);
}
