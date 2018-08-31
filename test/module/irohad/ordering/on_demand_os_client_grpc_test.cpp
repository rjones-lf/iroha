/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_os_client_grpc.hpp"

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "framework/mock_stream.h"
#include "interfaces/iroha_internal/proposal.hpp"
#include "ordering_mock.grpc.pb.h"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;

using grpc::testing::MockClientAsyncResponseReader;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;

struct OnDemandOsClientGrpcTest : public ::testing::Test {
  void SetUp() override {
    auto ustub = std::make_unique<proto::MockOnDemandOrderingStub>();
    stub = ustub.get();
    async_call =
        std::make_shared<network::AsyncGrpcClient<google::protobuf::Empty>>();
    client = std::make_shared<OnDemandOsClientGrpc>(
        std::move(ustub), async_call, [&] { return timepoint; }, timeout);
  }

  proto::MockOnDemandOrderingStub *stub;
  std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>> async_call;
  OnDemandOsClientGrpc::TimepointType timepoint;
  std::chrono::milliseconds timeout{1};
  std::shared_ptr<OnDemandOsClientGrpc> client;
};

/**
 * @given client
 * @when onTransactions is called
 * @then data is correctly serialized and sent
 */
TEST_F(OnDemandOsClientGrpcTest, onTransactions) {
  proto::TransactionsRequest request;
  auto r = std::make_unique<
      MockClientAsyncResponseReader<google::protobuf::Empty>>();
  EXPECT_CALL(*stub, AsyncSendTransactionsRaw(_, _, _))
      .WillOnce(DoAll(SaveArg<1>(&request), Return(r.get())));

  RoundType round;
  OdOsNotification::CollectionType collection;
  auto creator = "test";
  protocol::Transaction tx;
  tx.mutable_payload()->mutable_reduced_payload()->set_creator_account_id(
      creator);
  collection.push_back(std::make_unique<shared_model::proto::Transaction>(tx));
  client->onTransactions(round, std::move(collection));

  ASSERT_EQ(request.round().block_round(), round.first);
  ASSERT_EQ(request.round().reject_round(), round.second);
  ASSERT_EQ(request.transactions()
                .Get(0)
                .payload()
                .reduced_payload()
                .creator_account_id(),
            creator);
}

/**
 * Separate action required because ClientContext is non-copyable
 */
ACTION_P(SaveClientContextDeadline, deadline) {
  *deadline = arg0->deadline();
}

/**
 * @given client
 * @when onRequestProposal is called
 * AND proposal returned
 * @then data is correctly serialized and sent
 * AND reply is correctly deserialized
 */
TEST_F(OnDemandOsClientGrpcTest, onRequestProposal) {
  std::chrono::system_clock::time_point deadline;
  proto::ProposalRequest request;
  auto creator = "test";
  proto::ProposalResponse response;
  response.mutable_proposal()
      ->add_transactions()
      ->mutable_payload()
      ->mutable_reduced_payload()
      ->set_creator_account_id(creator);
  EXPECT_CALL(*stub, RequestProposal(_, _, _))
      .WillOnce(DoAll(SaveClientContextDeadline(&deadline),
                      SaveArg<1>(&request),
                      SetArgPointee<2>(response),
                      Return(grpc::Status::OK)));

  transport::RoundType round{1, 1};
  auto proposal = client->onRequestProposal(round);

  ASSERT_EQ(timepoint + timeout, deadline);
  ASSERT_EQ(request.round().block_round(), round.first);
  ASSERT_EQ(request.round().reject_round(), round.second);
  ASSERT_TRUE(proposal);
  ASSERT_EQ(proposal.value()->transactions()[0].creatorAccountId(), creator);
}

/**
 * @given client
 * @when onRequestProposal is called
 * AND no proposal returned
 * @then data is correctly serialized and sent
 * AND reply is correctly deserialized
 */
TEST_F(OnDemandOsClientGrpcTest, onRequestProposalNone) {
  std::chrono::system_clock::time_point deadline;
  proto::ProposalRequest request;
  proto::ProposalResponse response;
  EXPECT_CALL(*stub, RequestProposal(_, _, _))
      .WillOnce(DoAll(SaveClientContextDeadline(&deadline),
                      SaveArg<1>(&request),
                      SetArgPointee<2>(response),
                      Return(grpc::Status::OK)));

  transport::RoundType round{1, 1};
  auto proposal = client->onRequestProposal(round);

  ASSERT_EQ(timepoint + timeout, deadline);
  ASSERT_EQ(request.round().block_round(), round.first);
  ASSERT_EQ(request.round().reject_round(), round.second);
  ASSERT_FALSE(proposal);
}
