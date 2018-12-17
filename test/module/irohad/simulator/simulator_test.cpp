/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "simulator/impl/simulator.hpp"

#include <vector>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/find.hpp>
#include "backend/protobuf/proto_block_factory.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/proposal.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/cryptography/crypto_model_signer_mock.hpp"
#include "module/shared_model/validators/validators.hpp"

using namespace iroha;
using namespace iroha::validation;
using namespace iroha::ametsuchi;
using namespace iroha::simulator;
using namespace iroha::network;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::A;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnArg;

using wBlock = std::shared_ptr<shared_model::interface::Block>;

class SimulatorTest : public ::testing::Test {
 public:
  void SetUp() override {
    shared_model::crypto::crypto_signer_expecter =
        std::make_shared<shared_model::crypto::CryptoModelSignerExpecter>();

    validator = std::make_shared<MockStatefulValidator>();
    factory = std::make_shared<NiceMock<MockTemporaryFactory>>();
    query = std::make_shared<MockBlockQuery>();
    ordering_gate = std::make_shared<MockOrderingGate>();
    crypto_signer = std::make_shared<shared_model::crypto::CryptoModelSigner<>>(
        shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair());
    block_query_factory = std::make_shared<MockBlockQueryFactory>();
    EXPECT_CALL(*block_query_factory, createBlockQuery())
        .WillRepeatedly(testing::Return(boost::make_optional(
            std::shared_ptr<iroha::ametsuchi::BlockQuery>(query))));
    block_factory = std::make_unique<shared_model::proto::ProtoBlockFactory>(
        std::make_unique<shared_model::validation::MockValidator<
            shared_model::interface::Block>>(),
        std::make_unique<
            shared_model::validation::MockValidator<iroha::protocol::Block>>());
  }

  void TearDown() override {
    shared_model::crypto::crypto_signer_expecter.reset();
  }

  void init() {
    simulator = std::make_shared<Simulator>(ordering_gate,
                                            validator,
                                            factory,
                                            block_query_factory,
                                            crypto_signer,
                                            std::move(block_factory));
  }

  consensus::Round round;

  std::shared_ptr<MockStatefulValidator> validator;
  std::shared_ptr<MockTemporaryFactory> factory;
  std::shared_ptr<MockBlockQuery> query;
  std::shared_ptr<MockBlockQueryFactory> block_query_factory;
  std::shared_ptr<MockOrderingGate> ordering_gate;
  std::shared_ptr<shared_model::crypto::CryptoModelSigner<>> crypto_signer;
  std::unique_ptr<shared_model::interface::UnsafeBlockFactory> block_factory;

  std::shared_ptr<Simulator> simulator;
};

shared_model::proto::Block makeBlock(int height) {
  return TestBlockBuilder()
      .transactions(std::vector<shared_model::proto::Transaction>())
      .height(height)
      .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
      .build();
}

auto makeProposal(int height) {
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId("admin@ru")
                .addAssetQuantity("coin#coin", "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();
  std::vector<shared_model::proto::Transaction> txs = {tx, tx};
  auto proposal = shared_model::proto::ProposalBuilder()
                      .height(height)
                      .createdTime(iroha::time::now())
                      .transactions(txs)
                      .build();
  return std::make_shared<shared_model::proto::Proposal>(std::move(proposal));
}

TEST_F(SimulatorTest, ValidWhenInitialized) {
  // simulator constructor => onProposal subscription called
  EXPECT_CALL(*ordering_gate, onProposal())
      .WillOnce(Return(rxcpp::observable<>::empty<OrderingEvent>()));

  init();
}

TEST_F(SimulatorTest, ValidWhenPreviousBlock) {
  // proposal with height 2 => height 1 block present => new block generated
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId("admin@ru")
                .addAssetQuantity("coin#coin", "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();
  std::vector<shared_model::proto::Transaction> txs = {tx, tx};

  auto validation_result =
      std::make_unique<iroha::validation::VerifiedProposalAndErrors>();
  validation_result->verified_proposal =
      std::make_unique<shared_model::proto::Proposal>(
          shared_model::proto::ProposalBuilder()
              .height(2)
              .createdTime(iroha::time::now())
              .transactions(txs)
              .build());
  const auto &proposal = validation_result->verified_proposal;
  shared_model::proto::Block block = makeBlock(proposal->height() - 1);

  EXPECT_CALL(*factory, createTemporaryWsv()).Times(1);
  EXPECT_CALL(*query, getTopBlock())
      .WillOnce(Return(expected::makeValue(wBlock(clone(block)))));

  EXPECT_CALL(*query, getTopBlockHeight()).WillOnce(Return(block.height()));
  EXPECT_CALL(*validator, validate(_, _))
      .WillOnce(Invoke([&validation_result](const auto &p, auto &v) {
        return std::move(validation_result);
      }));

  EXPECT_CALL(*ordering_gate, onProposal())
      .WillOnce(Return(rxcpp::observable<>::empty<OrderingEvent>()));

  EXPECT_CALL(*shared_model::crypto::crypto_signer_expecter,
              sign(A<shared_model::interface::Block &>()))
      .Times(1);

  init();

  auto proposal_wrapper =
      make_test_subscriber<CallExact>(simulator->onVerifiedProposal(), 1);
  proposal_wrapper.subscribe([&proposal](auto event) {
    auto verified_proposal = getVerifiedProposalUnsafe(event);

    ASSERT_EQ(verified_proposal->verified_proposal->height(),
              proposal->height());
    ASSERT_EQ(verified_proposal->verified_proposal->transactions(),
              proposal->transactions());
    ASSERT_TRUE(verified_proposal->rejected_transactions.empty());
  });

  auto block_wrapper = make_test_subscriber<CallExact>(simulator->onBlock(), 1);
  block_wrapper.subscribe([&proposal](const auto &event) {
    auto block = getBlockUnsafe(event);

    ASSERT_EQ(block->height(), proposal->height());
    ASSERT_EQ(block->transactions(), proposal->transactions());
  });

  simulator->processProposal(*proposal, round);

  ASSERT_TRUE(proposal_wrapper.validate());
  ASSERT_TRUE(block_wrapper.validate());
}

TEST_F(SimulatorTest, FailWhenNoBlock) {
  // height 2 proposal => height 1 block not present => no validated proposal
  auto proposal = makeProposal(2);

  EXPECT_CALL(*factory, createTemporaryWsv()).Times(0);
  EXPECT_CALL(*query, getTopBlock())
      .WillOnce(Return(expected::makeError("no block")));

  EXPECT_CALL(*validator, validate(_, _)).Times(0);

  EXPECT_CALL(*ordering_gate, onProposal())
      .WillOnce(Return(rxcpp::observable<>::empty<OrderingEvent>()));

  EXPECT_CALL(*shared_model::crypto::crypto_signer_expecter,
              sign(A<shared_model::interface::Block &>()))
      .Times(0);

  init();

  auto proposal_wrapper =
      make_test_subscriber<CallExact>(simulator->onVerifiedProposal(), 0);
  proposal_wrapper.subscribe();

  auto block_wrapper = make_test_subscriber<CallExact>(simulator->onBlock(), 0);
  block_wrapper.subscribe();

  simulator->processProposal(*proposal, round);

  ASSERT_TRUE(proposal_wrapper.validate());
  ASSERT_TRUE(block_wrapper.validate());
}

TEST_F(SimulatorTest, FailWhenSameAsProposalHeight) {
  // proposal with height 2 => height 2 block present => no validated proposal
  auto proposal = makeProposal(2);

  auto block = makeBlock(proposal->height());

  EXPECT_CALL(*factory, createTemporaryWsv()).Times(0);

  EXPECT_CALL(*query, getTopBlock())
      .WillOnce(Return(expected::makeValue(wBlock(clone(block)))));

  EXPECT_CALL(*validator, validate(_, _)).Times(0);

  EXPECT_CALL(*ordering_gate, onProposal())
      .WillOnce(Return(rxcpp::observable<>::empty<OrderingEvent>()));

  EXPECT_CALL(*shared_model::crypto::crypto_signer_expecter,
              sign(A<shared_model::interface::Block &>()))
      .Times(0);

  init();

  auto proposal_wrapper =
      make_test_subscriber<CallExact>(simulator->onVerifiedProposal(), 0);
  proposal_wrapper.subscribe();

  auto block_wrapper = make_test_subscriber<CallExact>(simulator->onBlock(), 0);
  block_wrapper.subscribe();

  simulator->processProposal(*proposal, round);

  ASSERT_TRUE(proposal_wrapper.validate());
  ASSERT_TRUE(block_wrapper.validate());
}

/**
 * Checks, that after failing a certain number of transactions in a proposal,
 * returned verified proposal will have only valid transactions
 *
 * @given proposal consisting of several transactions
 * @when failing some of the transactions in that proposal
 * @then verified proposal consists of txs we did not fail, and the failed
 * transactions are provided as well
 */
TEST_F(SimulatorTest, SomeFailingTxs) {
  // create a 3-height proposal, but validator returns only a 2-height verified
  // proposal
  const int kNumTransactions = 3;
  std::vector<shared_model::proto::Transaction> txs;
  for (int i = 0; i < kNumTransactions; ++i) {
    txs.emplace_back(shared_model::proto::TransactionBuilder()
                         .createdTime(iroha::time::now() + i)
                         .creatorAccountId("admin@ru")
                         .addAssetQuantity("coin#coin", "1.0")
                         .quorum(1)
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair())
                         .finish());
  }
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      shared_model::proto::ProposalBuilder()
          .height(3)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .build());
  auto verified_proposal_and_errors =
      std::make_unique<VerifiedProposalAndErrors>();
  const shared_model::interface::types::HeightType verified_proposal_height = 2;
  const std::vector<shared_model::proto::Transaction>
      verified_proposal_transactions{txs[0]};
  verified_proposal_and_errors->verified_proposal =
      std::make_unique<shared_model::proto::Proposal>(
          shared_model::proto::ProposalBuilder()
              .height(verified_proposal_height)
              .createdTime(iroha::time::now())
              .transactions(verified_proposal_transactions)
              .build());
  for (auto rejected_tx = txs.begin() + 1; rejected_tx != txs.end();
       ++rejected_tx) {
    verified_proposal_and_errors->rejected_transactions.emplace_back(
        validation::TransactionError{
            rejected_tx->hash(),
            validation::CommandError{"SomeCommand", 1, "", true}});
  }
  shared_model::proto::Block block = makeBlock(proposal->height() - 1);

  EXPECT_CALL(*factory, createTemporaryWsv()).Times(1);
  EXPECT_CALL(*query, getTopBlock())
      .WillOnce(Return(expected::makeValue(wBlock(clone(block)))));

  EXPECT_CALL(*query, getTopBlockHeight()).WillOnce(Return(2));

  EXPECT_CALL(*validator, validate(_, _))
      .WillOnce(Invoke([&verified_proposal_and_errors](const auto &p, auto &v) {
        return std::move(verified_proposal_and_errors);
      }));

  EXPECT_CALL(*ordering_gate, onProposal())
      .WillOnce(Return(rxcpp::observable<>::empty<OrderingEvent>()));

  EXPECT_CALL(*shared_model::crypto::crypto_signer_expecter,
              sign(A<shared_model::interface::Block &>()))
      .Times(1);

  init();

  auto proposal_wrapper =
      make_test_subscriber<CallExact>(simulator->onVerifiedProposal(), 1);
  proposal_wrapper.subscribe([&](auto event) {
    auto verified_proposal_ = getVerifiedProposalUnsafe(event);

    // ensure that txs in verified proposal do not include failed ones
    ASSERT_EQ(verified_proposal_->verified_proposal->height(),
              verified_proposal_height);
    ASSERT_EQ(verified_proposal_->verified_proposal->transactions(),
              verified_proposal_transactions);
    ASSERT_TRUE(verified_proposal_->rejected_transactions.size()
                == kNumTransactions - 1);
    const auto verified_proposal_rejected_tx_hashes =
        verified_proposal_->rejected_transactions
        | boost::adaptors::transformed(
              [](const auto &tx_error) { return tx_error.tx_hash; });
    for (auto rejected_tx = txs.begin() + 1; rejected_tx != txs.end();
         ++rejected_tx) {
      ASSERT_NE(boost::range::find(verified_proposal_rejected_tx_hashes,
                                   rejected_tx->hash()),
                boost::end(verified_proposal_rejected_tx_hashes))
          << rejected_tx->toString() << " missing in rejected transactions.";
    }
  });

  simulator->processProposal(*proposal, round);

  ASSERT_TRUE(proposal_wrapper.validate());
}
