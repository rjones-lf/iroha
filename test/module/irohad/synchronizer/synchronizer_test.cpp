/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>

#include "backend/protobuf/block.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "synchronizer/impl/synchronizer_impl.hpp"
#include "validation/chain_validator.hpp"
#include "validators/answer.hpp"

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

class MockBlockValidator {
 public:
  MOCK_CONST_METHOD1(
      validate,
      shared_model::validation::Answer(const shared_model::interface::Block &));
};

template <typename T = MockBlockValidator>
class TemplateMockBlockValidator {
 public:
  std::shared_ptr<T> validator;
  TemplateMockBlockValidator() : validator(std::make_shared<T>()) {}
  shared_model::validation::Answer validate(
      const shared_model::interface::Block &block) const {
    return validator->validate(block);
  }
};

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
  }

  void init() {
    synchronizer = std::make_shared<SynchronizerImpl>(consensus_gate,
                                                      chain_validator,
                                                      mutable_factory,
                                                      block_query_factory,
                                                      block_loader);
    ON_CALL(*block_query_factory, createBlockQuery())
        .WillByDefault(Return(boost::make_optional(
            std::shared_ptr<iroha::ametsuchi::BlockQuery>(block_query))));
  }

  Commit makeCommit(size_t time = iroha::time::now(),
                    PeerVotedFor type = PeerVotedFor::kOtherBlock) const {
    auto block = TestUnsignedBlockBuilder()
                     .height(commit_height)
                     .createdTime(time)
                     .build()
                     .signAndAddSignature(
                         shared_model::crypto::DefaultCryptoAlgorithmType::
                             generateKeypair())
                     .finish();
    return {std::make_shared<shared_model::proto::Block>(std::move(block)),
            type};
  }

  std::shared_ptr<MockChainValidator> chain_validator;
  std::shared_ptr<MockMutableFactory> mutable_factory;
  std::shared_ptr<MockBlockQueryFactory> block_query_factory;
  std::shared_ptr<MockBlockLoader> block_loader;
  std::shared_ptr<MockConsensusGate> consensus_gate;
  std::shared_ptr<MockBlockQuery> block_query;

  const shared_model::interface::types::HeightType commit_height{5};

  std::shared_ptr<SynchronizerImpl> synchronizer;
};

TEST_F(SynchronizerTest, ValidWhenInitialized) {
  // synchronizer constructor => on_commit subscription called
  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();
}

/**
 * @given A commit from consensus and initialized components
 * @when a valid block that can be applied
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenSingleCommitSynchronized) {
  std::shared_ptr<shared_model::interface::Block> test_block =
      std::make_shared<shared_model::proto::Block>(
          TestBlockBuilder().height(5).build());
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      test_blocks = rxcpp::observable<>::just(test_block);

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).WillOnce(Return(true));

  EXPECT_CALL(*block_loader, retrieveBlocks(_, _)).Times(0);

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([test_block](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([test_block](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), test_block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(Commit{test_block, PeerVotedFor::kOtherBlock});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when Storage cannot be initialized
 * @then No commit should be passed
 */
TEST_F(SynchronizerTest, ValidWhenBadStorage) {
  std::shared_ptr<shared_model::interface::Block> test_block =
      std::make_shared<shared_model::proto::Block>(TestBlockBuilder().build());

  DefaultValue<
      expected::Result<std::unique_ptr<MutableStorage>, std::string>>::Clear();
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(0);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);

  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).Times(0);

  EXPECT_CALL(*block_loader, retrieveBlocks(_, _)).Times(0);

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 0);
  wrapper.subscribe();

  synchronizer->process_commit(Commit{test_block, PeerVotedFor::kOtherBlock});

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when A valid chain with expected ending
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenValidChain) {
  auto commit_message = makeCommit();
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      commit_message_blocks = rxcpp::observable<>::just(commit_message.block);

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);

  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateAndApply(_, _))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillOnce(Return(commit_message_blocks));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([commit_message](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([commit_message](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message.block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(commit_message);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A valid block that cannot be applied directly
 * @when process_commit is called
 * @then observable of retrieveBlocks must be evaluated four times:
 *   - to validate whole chain
 *   - to validate last block of chain (x2)
 *   - to create a vector
 */
TEST_F(SynchronizerTest, ExactlyThreeRetrievals) {
  auto commit_message = makeCommit();

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);
  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);
  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));
  EXPECT_CALL(*chain_validator, validateAndApply(_, _))
      .WillOnce(Return(false))
      .WillOnce(testing::Invoke([](auto chain, auto &) {
        // emulate chain check
        chain.as_blocking().subscribe([](auto) {});
        return true;
      }));
  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillOnce(Return(rxcpp::observable<>::create<std::shared_ptr<
                           shared_model::interface::Block>>([commit_message](
                                                                auto s) {
        static int times = 0;
        if (times++ > 4) {
          FAIL() << "Observable of retrieveBlocks must be evaluated four times";
        }
        s.on_next(commit_message.block);
        s.on_completed();
      })));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe();

  synchronizer->process_commit(commit_message);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given commit from the consensus and initialized components
 * @when synchronizer fails to download block from some peer
 * @then it will try until success
 */
TEST_F(SynchronizerTest, RetrieveBlockTwoFailures) {
  auto commit_message = makeCommit();
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      commit_message_blocks = rxcpp::observable<>::just(commit_message.block);

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*block_loader, retrieveBlocks(_, _))
      .WillRepeatedly(Return(commit_message_blocks));

  // fail the chain validation two times so that synchronizer will try more
  EXPECT_CALL(*chain_validator, validateAndApply(_, _))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([commit_message](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([commit_message](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message.block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(commit_message);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given commit with the block peer voted for
 * @when synchronizer processes the commit
 * @then commitPrepared is called @and commit is not called
 */
TEST_F(SynchronizerTest, VotedForBlockCommitPrepared) {
  auto commit_message =
      makeCommit(iroha::time::now(), PeerVotedFor::kThisBlock);
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      commit_message_blocks = rxcpp::observable<>::just(commit_message.block);

  EXPECT_CALL(*mutable_factory, commitPrepared(_)).WillOnce(Return(true));

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([commit_message](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([commit_message](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message.block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(commit_message);
}

/**
 * @given commit with the block which is different than the peer has voted for
 * @when synchronizer processes the commit
 * @then commitPrepared is not called @and commit is called
 */
TEST_F(SynchronizerTest, VotedForOtherCommitPrepared) {
  auto commit_message = makeCommit();
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      commit_message_blocks = rxcpp::observable<>::just(commit_message.block);

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);

  EXPECT_CALL(*mutable_factory, commitPrepared(_)).Times(0);

  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).WillOnce(Return(true));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([commit_message](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([commit_message](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message.block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(commit_message);
}

/**
 * @given commit with the block peer voted for
 * @when synchronizer processes the commit @and commit prepared is unsuccessful
 * @then commit is called and synchronizer works as expected
 */
TEST_F(SynchronizerTest, VotedForThisCommitPreparedFailure) {
  auto commit_message =
      makeCommit(iroha::time::now(), PeerVotedFor::kThisBlock);
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      commit_message_blocks = rxcpp::observable<>::just(commit_message.block);

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);

  EXPECT_CALL(*mutable_factory, commitPrepared(_)).WillOnce(Return(false));

  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateAndApply(_, _)).WillOnce(Return(true));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<network::Commit>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([commit_message](auto commit_event) {
    auto block_wrapper =
        make_test_subscriber<CallExact>(commit_event.synced_blocks, 1);
    block_wrapper.subscribe([commit_message](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), commit_message.block->height());
    });
    ASSERT_EQ(commit_event.sync_outcome, SynchronizationOutcomeType::kCommit);
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(commit_message);
}
