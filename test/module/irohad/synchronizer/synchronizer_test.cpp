/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <gmock/gmock.h>

#include "backend/protobuf/block.hpp"
#include "builders/protobuf/block.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
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
    block_loader = std::make_shared<MockBlockLoader>();
    consensus_gate = std::make_shared<MockConsensusGate>();
  }

  void init() {
    synchronizer = std::make_shared<SynchronizerImpl>(
        consensus_gate, chain_validator, mutable_factory, block_loader);
  }

  std::shared_ptr<MockChainValidator> chain_validator;
  std::shared_ptr<MockMutableFactory> mutable_factory;
  std::shared_ptr<MockBlockLoader> block_loader;
  std::shared_ptr<MockConsensusGate> consensus_gate;

  std::shared_ptr<SynchronizerImpl> synchronizer;
};

TEST_F(SynchronizerTest, ValidWhenInitialized) {
  // synchronizer constructor => on_commit subscription called
  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));

  init();
}


/**
 * @given A commit from consensus and initialized components
 * @when a valid block that can be applied
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenSingleCommitSynchronized) {
  auto block = TestBlockBuilder().height(5).build();
  std::shared_ptr<shared_model::interface::Block> test_block =
      std::make_shared<shared_model::proto::Block>(std::move(block));

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateBlock(testing::Ref(*test_block), _))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_loader, retrieveBlocks(_)).Times(0);

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([test_block](auto commit) {
    auto block_wrapper = make_test_subscriber<CallExact>(commit, 1);
    block_wrapper.subscribe([test_block](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), test_block->height());
    });
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(test_block);

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
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(1);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);

  EXPECT_CALL(*chain_validator, validateBlock(_, _)).Times(0);

  EXPECT_CALL(*block_loader, retrieveBlocks(_)).Times(0);

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 0);
  wrapper.subscribe();

  synchronizer->process_commit(test_block);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when A valid chain with expected ending
 * @then Successful commit
 */
TEST_F(SynchronizerTest, ValidWhenValidChain) {
  TemplateMockBlockValidator<MockBlockValidator> mockBlockValidator;
  EXPECT_CALL(*mockBlockValidator.validator, validate(_))
      .WillOnce(Return(shared_model::validation::Answer()));
  using TestUnsignedBlockBuilder = shared_model::proto::TemplateBlockBuilder<
      (1 << shared_model::proto::TemplateBlockBuilder<>::total) - 1,
      TemplateMockBlockValidator<MockBlockValidator>,
      shared_model::proto::UnsignedWrapper<shared_model::proto::Block>>;

  auto block = TestUnsignedBlockBuilder(mockBlockValidator)
                   .height(5)
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());
  std::shared_ptr<shared_model::interface::Block> test_block =
      std::make_shared<shared_model::proto::Block>(std::move(block));

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(2);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(1);

  EXPECT_CALL(*chain_validator, validateBlock(testing::Ref(*test_block), _))
      .WillOnce(Return(false));

  EXPECT_CALL(*chain_validator, validateChain(_, _)).WillOnce(Return(true));

  EXPECT_CALL(*block_loader, retrieveBlocks(_))
      .WillOnce(Return(rxcpp::observable<>::just(test_block)));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 1);
  wrapper.subscribe([test_block](auto commit) {
    auto block_wrapper = make_test_subscriber<CallExact>(commit, 1);
    block_wrapper.subscribe([test_block](auto block) {
      // Check commit block
      ASSERT_EQ(block->height(), test_block->height());
    });
    ASSERT_TRUE(block_wrapper.validate());
  });

  synchronizer->process_commit(test_block);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given A commit from consensus and initialized components
 * @when A valid chain with unexpected ending
 * @then No commit should be passed
 */
TEST_F(SynchronizerTest, InvalidWhenUnexpectedEnd) {
  TemplateMockBlockValidator<MockBlockValidator> mockBlockValidator;
  EXPECT_CALL(*mockBlockValidator.validator, validate(_))
      .WillRepeatedly(Return(shared_model::validation::Answer()));
  using TestUnsignedBlockBuilder = shared_model::proto::TemplateBlockBuilder<
      (1 << shared_model::proto::TemplateBlockBuilder<>::total) - 1,
      TemplateMockBlockValidator<MockBlockValidator>,
      shared_model::proto::UnsignedWrapper<shared_model::proto::Block>>;

  auto block = TestUnsignedBlockBuilder(mockBlockValidator)
                   .height(5)
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());
  std::shared_ptr<shared_model::interface::Block> test_block =
      std::make_shared<shared_model::proto::Block>(std::move(block));

  DefaultValue<expected::Result<std::unique_ptr<MutableStorage>, std::string>>::
      SetFactory(&createMockMutableStorage);
  EXPECT_CALL(*mutable_factory, createMutableStorage()).Times(2);

  EXPECT_CALL(*mutable_factory, commit_(_)).Times(0);

  EXPECT_CALL(*chain_validator, validateBlock(testing::Ref(*test_block), _))
      .WillOnce(Return(false));

  EXPECT_CALL(*chain_validator, validateChain(_, _)).WillOnce(Return(true));

  // wrong block has different hash
  auto wrong_block_end =
      TestUnsignedBlockBuilder(mockBlockValidator)
          .height(5)
          .createdTime(iroha::time::now())
          .build()
          .signAndAddSignature(
              shared_model::crypto::DefaultCryptoAlgorithmType::
                  generateKeypair());
  std::shared_ptr<shared_model::interface::Block> wrong_test_block =
      std::make_shared<shared_model::proto::Block>(std::move(wrong_block_end));

  EXPECT_CALL(*block_loader, retrieveBlocks(_))
      .WillOnce(Return(rxcpp::observable<>::just(wrong_test_block)));

  EXPECT_CALL(*consensus_gate, on_commit())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));

  init();

  auto wrapper =
      make_test_subscriber<CallExact>(synchronizer->on_commit_chain(), 0);
  wrapper.subscribe();

  synchronizer->process_commit(test_block);

  ASSERT_TRUE(wrapper.validate());
}
