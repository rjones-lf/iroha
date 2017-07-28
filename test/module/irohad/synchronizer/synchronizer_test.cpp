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
#include <gtest/gtest.h>
#include "synchronizer/impl/synchronizer_impl.hpp"
#include "validation/chain_validator.hpp"

using iroha::ametsuchi::MutableFactory;
using iroha::ametsuchi::MutableStorage;
using iroha::model::Block;
using iroha::model::Peer;

using ::testing::Return;
using ::testing::_;

class ChainValidatorMock : public iroha::validation::ChainValidator {
 public:
  MOCK_METHOD2(validateChain, bool(rxcpp::observable<Block>&, MutableStorage&));

  MOCK_METHOD2(validateBlock, bool(const Block&, MutableStorage&));
};

class MutableFactoryMock : public MutableFactory {
 public:
  virtual std::unique_ptr<MutableStorage> createMutableStorage() {
    return std::unique_ptr<MutableStorage>(createMutableStorageProxy());
  }

  MOCK_METHOD0(createMutableStorageProxy, MutableStorage*());

  virtual void commit(std::unique_ptr<MutableStorage> a) {
    commitProxy(a.get());
  }
  MOCK_METHOD1(commitProxy, void(MutableStorage*));
};

class BlockLoader : public iroha::network::BlockLoader {
 public:
  MOCK_METHOD2(requestBlocks, rxcpp::observable<Block>(Peer&, Block&));
};

TEST(SynchronizerTest, CommitProccesing) {
  ChainValidatorMock chain_validator;
  MutableFactoryMock mutable_factory;
  BlockLoader block_loader;

  auto synchronizer = iroha::synchronizer::SynchronizerImpl(
      chain_validator, mutable_factory, block_loader);

  auto test_block = Block();
  test_block.height = 5;

  EXPECT_CALL(mutable_factory, createMutableStorageProxy())
      .WillRepeatedly(Return(nullptr));

  EXPECT_CALL(chain_validator, validateBlock(_, _))
      .WillRepeatedly(Return(true));

  synchronizer.process_commit(test_block);

  synchronizer.on_commit_chain().subscribe([&test_block](auto commit_chain) {
    commit_chain.subscribe([&test_block](auto commit_block) {
      // Check commit block
      ASSERT_EQ(commit_block.height, test_block.height);
    });
  });
}
