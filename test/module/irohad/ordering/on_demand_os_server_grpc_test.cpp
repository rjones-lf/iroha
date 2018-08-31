/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_os_server_grpc.hpp"

#include <gtest/gtest.h>
#include "backend/protobuf/proposal.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;

using ::testing::_;
using ::testing::ByMove;
using ::testing::Return;

struct OnDemandOsServerGrpcTest : public ::testing::Test {
  void SetUp() override {
    notification = std::make_shared<MockOdOsNotification>();
    server = std::make_shared<OnDemandOsServerGrpc>(notification);
  }

  std::shared_ptr<MockOdOsNotification> notification;
  std::shared_ptr<OnDemandOsServerGrpc> server;
};

/**
 * Separate action required because CollectionType is non-copyable
 */
ACTION_P(SaveArg1Move, var) {
  *var = std::move(arg1);
}

/**
 * @given server
 * @when collection is received from the network
 * @then it is correctly deserialized and passed
 */
TEST_F(OnDemandOsServerGrpcTest, SendTransactions) {
  OdOsNotification::CollectionType collection;
  auto creator = "test";
  RoundType round{1, 1};

  EXPECT_CALL(*notification, onTransactions(round, _))
      .WillOnce(SaveArg1Move(&collection));
  proto::TransactionsRequest request;
  request.mutable_round()->set_block_round(round.first);
  request.mutable_round()->set_reject_round(round.second);
  request.add_transactions()
      ->mutable_payload()
      ->mutable_reduced_payload()
      ->set_creator_account_id(creator);

  server->SendTransactions(nullptr, &request, nullptr);

  ASSERT_EQ(collection.at(0)->creatorAccountId(), creator);
}

/**
 * @given server
 * @when proposal is requested
 * AND proposal returned
 * @then it is correctly serialized
 */
TEST_F(OnDemandOsServerGrpcTest, RequestProposal) {
  auto creator = "test";
  transport::RoundType round{1, 1};
  proto::ProposalRequest request;
  request.mutable_round()->set_block_round(round.first);
  request.mutable_round()->set_reject_round(round.second);
  proto::ProposalResponse response;
  protocol::Proposal proposal;
  proposal.add_transactions()
      ->mutable_payload()
      ->mutable_reduced_payload()
      ->set_creator_account_id(creator);

  std::unique_ptr<shared_model::interface::Proposal> iproposal(
      std::make_unique<shared_model::proto::Proposal>(proposal));
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(iproposal))));

  server->RequestProposal(nullptr, &request, &response);

  ASSERT_TRUE(response.has_proposal());
  ASSERT_EQ(response.proposal()
                .transactions()
                .Get(0)
                .payload()
                .reduced_payload()
                .creator_account_id(),
            creator);
}

/**
 * @given server
 * @when proposal is requested
 * AND no proposal returned
 * @then the result is correctly serialized
 */
TEST_F(OnDemandOsServerGrpcTest, RequestProposalNone) {
  transport::RoundType round{1, 1};
  proto::ProposalRequest request;
  request.mutable_round()->set_block_round(round.first);
  request.mutable_round()->set_reject_round(round.second);
  proto::ProposalResponse response;
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(boost::none))));

  server->RequestProposal(nullptr, &request, &response);

  ASSERT_FALSE(response.has_proposal());
}
