/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <gmock/gmock.h>
#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/block.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "validation/chain_validator.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::synchronizer;
using namespace iroha::validation;
using namespace iroha::network;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::ByMove;
using ::testing::DefaultValue;
using ::testing::Return;

class SynchronizerTest : public ::testing::Test {
 public:
  void SetUp() override {
    chain_validator = std::make_shared<MockChainValidator>();
    mutable_factory = std::make_shared<MockMutableFactory>();
    block_query_factory =
        std::make_shared<::testing::NiceMock<MockBlockQueryFactory>>();
    block_loader = std::make_shared<MockBlockLoader>();
    consensus_gate = std::make_shared<MockConsensusGate>();
    block_query = std::make_shared<::testing::NiceMock<MockBlockQuery>>();

    commit_message = makeCommit();
    public_keys = boost::copy_range<
        shared_model::interface::types::PublicKeyCollectionType>(
        commit_message->signatures()
        | boost::adaptors::transformed(
              [](auto &signature) { return signature.publicKey(); }));
    hash = commit_message->hash();

    EXPECT_CALL(*consensus_gate, onOutcome())
        .WillOnce(Return(gate_outcome.get_observable()));

    ON_CALL(*block_query_factory, createBlockQuery())
        .WillByDefault(Return(boost::make_optional(
            std::shared_ptr<iroha::ametsuchi::BlockQuery>(block_query))));
    ON_CALL(*block_query, getTopBlockHeight())
        .WillByDefault(Return(kHeight - 1));

    synchronizer = std::make_shared<SynchronizerImpl>(consensus_gate,
                                                      chain_validator,
                                                      mutable_factory,
                                                      block_query_factory,
                                                      block_loader);
  }

  std::shared_ptr<shared_model::interface::Block> makeCommit(
      size_t time = iroha::time::now()) const {
    auto block = TestUnsignedBlockBuilder()
                     .height(kHeight)
                     .createdTime(time)
                     .build()
                     .signAndAddSignature(
                         shared_model::crypto::DefaultCryptoAlgorithmType::
                             generateKeypair())
                     .finish();
    return std::make_shared<shared_model::proto::Block>(std::move(block));
  }

  const shared_model::interface::types::HeightType kHeight{5};

  std::shared_ptr<MockChainValidator> chain_validator;
  std::shared_ptr<MockMutableFactory> mutable_factory;
  std::shared_ptr<MockBlockQueryFactory> block_query_factory;
  std::shared_ptr<MockBlockLoader> block_loader;
  std::shared_ptr<MockConsensusGate> consensus_gate;
  std::shared_ptr<MockBlockQuery> block_query;

  std::shared_ptr<shared_model::interface::Block> commit_message;
  shared_model::interface::types::PublicKeyCollectionType public_keys;
  shared_model::interface::types::HashType hash;

  rxcpp::subjects::subject<ConsensusGate::GateObject> gate_outcome;

  std::shared_ptr<SynchronizerImpl> synchronizer;
};

/**
 * @given A commit from consensus and initialized components
 * @when a valid block that can be applied
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenSingleCommitSynchronized) {
  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);
  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);
  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).Times(0);
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _)).Times(0);

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::PairValid{commit_message, consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when Storage cannot be initialized
 * @then No commit should be passed
 */
TEST_F(SynchronizerTest, ValidWhenBadStorage) {
  DefaultValue<
      expected::Result<std::unique_ptr<MutableStorage>, std::string>>::Clear();
  EXPECT_CALL(*mutable_factory, createMutableStorage())
      .WillOnce(Return(ByMove(expected::makeError("Connection was closed"))));
  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);
  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).Times(0);
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _)).Times(0);

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 0);
  wrapper.subscribe();

  gate_outcome.get_subscriber().on_next(
      consensus::PairValid{commit_message, consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when gate have voted for other block
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenValidChain) {
  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);

  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);
  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillOnce(Return(rxcpp::observable<>::just(commit_message)));

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::VoteOther{public_keys, hash, consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when gate have voted for other block
 * @then retrieveBlocks called again after unsuccessful download attempt
 */
TEST_F(SynchronizerTest, ExactlyThreeRetrievals) {
  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);
  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);
  EXPECT_CALL(*chain_validator, validateAndApply(_, _))
      .WillOnce(Return(false))
      .WillOnce(testing::Invoke([](auto chain, auto &) {
        // emulate chain check
        chain.as_blocking().subscribe([](auto) {});
        return true;
      }));
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()))
      .WillOnce(Return(rxcpp::observable<>::just(commit_message)))
      .WillOnce(Return(rxcpp::observable<>::just(commit_message)));

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe();

  gate_outcome.get_subscriber().on_next(
      consensus::VoteOther{public_keys, hash, consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given commit from the consensus and initialized components
 * @when synchronizer fails to download block from some peer
 * @then it will try until success
 */
TEST_F(SynchronizerTest, RetrieveBlockTwoFailures) {
  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);
  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillRepeatedly(Return(rxcpp::observable<>::just(commit_message)));

  // fail the chain validation two times so that synchronizer will try more
  EXPECT_CALL(*chain_validator, validateAndApply(_, _))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::VoteOther{public_keys, hash, consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized components
 * @when gate have got reject on proposal
 * @then synchronizer output is also reject
 */
TEST_F(SynchronizerTest, ProposalRejectOutcome) {
  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 0);
    block_wrapper.subscribe();
    ASSERT_TRUE(block_wrapper.validate());
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kReject);
  });

  gate_outcome.get_subscriber().on_next(
      consensus::ProposalReject{consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized components
 * @when gate have got reject on block
 * @then synchronizer output is also reject
 */
TEST_F(SynchronizerTest, BlockRejectOutcome) {
  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 0);
    block_wrapper.subscribe();
    ASSERT_TRUE(block_wrapper.validate());
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kReject);
  });

  gate_outcome.get_subscriber().on_next(
      consensus::BlockReject{consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given initialized components
 * @when gate have got agreement on none
 * @then synchronizer output is also none
 */
TEST_F(SynchronizerTest, NoneOutcome) {
  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 0);
    block_wrapper.subscribe();
    ASSERT_TRUE(block_wrapper.validate());
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kNothing);
  });

  gate_outcome.get_subscriber().on_next(
      consensus::AgreementOnNone{consensus::Round{kHeight, 1}});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given commit with the block peer voted for
 * @when synchronizer processes the commit
 * @then commitPrepared is called @and commit is not called
 */
TEST_F(SynchronizerTest, VotedForBlockCommitPrepared) {
  EXPECT_CALL(*mutable_factory, commitPrepared(_)).WillOnce(Return(true));

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::PairValid{commit_message, consensus::Round{kHeight, 1}});
}

/**
 * @given commit with the block which is different than the peer has voted for
 * @when synchronizer processes the commit
 * @then commitPrepared is not called @and commit is called
 */
TEST_F(SynchronizerTest, VotedForOtherCommitPrepared) {
  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);

  EXPECT_CALL(*mutable_factory, commitPrepared(_)).Times(0);

  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillRepeatedly(Return(rxcpp::observable<>::just(commit_message)));

  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).WillOnce(Return(true));

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::VoteOther{public_keys, hash, consensus::Round{kHeight, 1}});
}

/**
 * @given commit with the block peer voted for
 * @when synchronizer processes the commit @and commit prepared is unsuccessful
 * @then commit is called and synchronizer works as expected
 */
TEST_F(SynchronizerTest, VotedForThisCommitPreparedFailure) {
  auto ustorage = std::make_unique<MockMutableStorage>();

  auto storage = ustorage.get();

  auto storage_value =
      expected::makeValue<std::unique_ptr<MutableStorage>>(std::move(ustorage));

  EXPECT_CALL(*mutable_factory, commitPrepared(_)).WillOnce(Return(false));

  EXPECT_CALL(*mutable_factory, createMutableStorage())
      .WillOnce(Return(ByMove(std::move(storage_value))));

  EXPECT_CALL(*storage, apply(_)).WillOnce(Return(true));

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([this](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([this](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  gate_outcome.get_subscriber().on_next(
      consensus::PairValid{commit_message, consensus::Round{kHeight, 1}});
}
