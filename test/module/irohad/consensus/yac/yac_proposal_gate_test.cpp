/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_proposal_gate_impl.hpp"

#include "framework/result_fixture.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::network;
using namespace iroha::consensus::yac;
using namespace framework::test_subscriber;
using namespace framework::expected;
using namespace shared_model::crypto;

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ReturnRef;

struct YacProposalGateTest : public ::testing::Test {
  void SetUp() override {
    proposal = std::make_unique<MockProposal>();
    EXPECT_CALL(*proposal, blob()).WillRepeatedly(ReturnRef(blob));
    hash = proposal->hash();

    info.hash = hash;
    info.round = round;

    auto uhash_gate = std::make_unique<MockHashGate>();
    hash_gate = uhash_gate.get();
    EXPECT_CALL(*hash_gate, onOutcome())
        .WillRepeatedly(Return(outcome_subject.get_observable()));
    auto upeer_orderer = std::make_unique<MockYacPeerOrderer>();
    peer_orderer = upeer_orderer.get();
    hash_provider = std::make_shared<MockYacProposalHashProvider>();

    gate = std::make_shared<YacProposalGateImpl>(
        std::move(uhash_gate), std::move(upeer_orderer), hash_provider);
  }

  shared_model::interface::types::BlobType blob;
  shared_model::interface::types::HashType hash;
  std::unique_ptr<MockProposal> proposal;
  iroha::ordering::transport::RoundType round{1, 1};
  YacHash yac_hash{"proposal", "block"};
  ClusterOrdering order = ClusterOrdering::create({mk_peer("node")}).value();
  VoteMessage message{yac_hash, {}};
  YacProposalHashProvider::ProposalInfo info;

  rxcpp::subjects::subject<Answer> outcome_subject;

  MockHashGate *hash_gate;
  MockYacPeerOrderer *peer_orderer;
  std::shared_ptr<MockYacProposalHashProvider> hash_provider;

  std::shared_ptr<ProposalGate> gate;
};

/**
 * @given yac proposal gate with dependencies
 * @when cluster order cannot be produced
 * @then no voting is done
 */
TEST_F(YacProposalGateTest, NoClusterOrder) {
  // make hash from proposal vote
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(yac_hash));

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_)).WillOnce(Return(boost::none));

  // ya consensus
  EXPECT_CALL(*hash_gate, vote(_, _)).Times(0);

  std::unique_ptr<shared_model::interface::Proposal> iproposal(
      std::move(proposal));
  auto result = gate->vote(ProposalVote{std::move(iproposal), round});

  ASSERT_TRUE(err(result));
}

/**
 * @given yac proposal gate with dependencies
 * @when commit achieved for current proposal
 * @then commit emitted
 */
TEST_F(YacProposalGateTest, CommitAchieved) {
  // make hash from proposal vote
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(yac_hash));

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_)).WillOnce(Return(order));

  // ya consensus
  EXPECT_CALL(*hash_gate, vote(yac_hash, _)).Times(1);

  // convert yac hash to proposal info
  EXPECT_CALL(*hash_provider, makeProposalInfo(yac_hash))
      .WillOnce(Return(info));

  // commit emitted
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->outcomes(), 1);
  gate_wrapper.subscribe([this](const auto &outcome) {
    ASSERT_NO_THROW({
      auto commit = boost::apply_visitor(
          framework::SpecifiedVisitor<ProposalCommit>(), outcome);
      ASSERT_TRUE(commit.proposal);
      ASSERT_EQ(hash, commit.proposal.value()->hash());
      ASSERT_EQ(round, commit.round);
    });
  });

  std::unique_ptr<shared_model::interface::Proposal> iproposal(
      std::move(proposal));
  auto result = gate->vote(ProposalVote{std::move(iproposal), round});
  outcome_subject.get_subscriber().on_next(Answer(CommitMessage({message})));

  ASSERT_TRUE(val(result));
  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given yac proposal gate with dependencies
 * @when reject achieved for current proposal
 * @then reject emitted
 */
TEST_F(YacProposalGateTest, RejectAchieved) {
  // ya consensus
  EXPECT_CALL(*hash_gate, vote(yac_hash, _)).Times(1);

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_)).WillOnce(Return(order));

  // make hash from proposal vote
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(yac_hash));

  // convert yac hash to proposal info
  EXPECT_CALL(*hash_provider, makeProposalInfo(yac_hash))
      .WillOnce(Return(info));

  // reject emitted
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->outcomes(), 1);
  gate_wrapper.subscribe([this](const auto &outcome) {
    ASSERT_NO_THROW({
      auto reject = boost::apply_visitor(
          framework::SpecifiedVisitor<ProposalReject>(), outcome);
      ASSERT_EQ(round, reject.round);
    });
  });

  std::unique_ptr<shared_model::interface::Proposal> iproposal(
      std::move(proposal));
  auto result = gate->vote(ProposalVote{std::move(iproposal), round});
  outcome_subject.get_subscriber().on_next(Answer(RejectMessage({message})));

  ASSERT_TRUE(val(result));
  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given yac proposal gate with dependencies
 * @when commit achieved for different proposal
 * @then commit is loaded and emitted
 * TODO andrei 30.07.2018 IR-1435 proposal consensus cache
 */
TEST_F(YacProposalGateTest, DifferentCommitAchieved) {}
