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

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/model/model_mocks.hpp"
#include "validation/impl/chain_validator_impl.hpp"

using namespace iroha;
using namespace iroha::model;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

using ::testing::_;
using ::testing::A;
using ::testing::Return;
using ::testing::InvokeArgument;
using ::testing::ByRef;

class ChainValidationTest : public ::testing::Test {
 public:
  ChainValidatorImpl validator;
  MockMutableStorage storage;
};

TEST_F(ChainValidationTest, ValidWhenOnePeer) {
  EXPECT_CALL(storage, getPeers())
      .WillOnce(Return(std::vector<model::Peer>(1)));

  Block block;
  block.sigs.emplace_back();

  EXPECT_CALL(storage, apply(block, _)).WillOnce(Return(true));

  ASSERT_TRUE(validator.validateBlock(block, storage));
}

TEST_F(ChainValidationTest, ValidWhenNoPeers) {
  EXPECT_CALL(storage, getPeers())
      .WillOnce(Return(std::vector<model::Peer>(0)));

  Block block;
  block.sigs.emplace_back();

  EXPECT_CALL(storage, apply(block, _)).WillOnce(Return(true));

  ASSERT_TRUE(validator.validateBlock(block, storage));
}

TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  MockWsvCommand wsvCommand;

  EXPECT_CALL(storage, getPeers())
      .WillOnce(Return(std::vector<model::Peer>(1)));

  auto cmd = std::make_shared<MockCommand>();

  Block block;
  block.sigs.emplace_back();
  block.transactions.emplace_back();
  block.transactions.front().commands.emplace_back(cmd);
  block.prev_hash.fill(1);

  hash256_t myhash;
  myhash.fill(0);

  EXPECT_CALL(storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(block), ByRef(storage), ByRef(myhash)));

  ASSERT_FALSE(validator.validateBlock(block, storage));
}

TEST_F(ChainValidationTest, ValidWhenSamePrevHash) {
  MockWsvCommand wsvCommand;

  EXPECT_CALL(storage, getPeers())
      .WillOnce(Return(std::vector<model::Peer>(1)));

  auto cmd = std::make_shared<MockCommand>();

  Block block;
  block.sigs.emplace_back();
  block.transactions.emplace_back();
  block.transactions.front().commands.emplace_back(cmd);
  block.prev_hash.fill(0);

  hash256_t myhash;
  myhash.fill(0);

  EXPECT_CALL(storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(block), ByRef(storage), ByRef(myhash)));

  ASSERT_TRUE(validator.validateBlock(block, storage));
}

TEST_F(ChainValidationTest, ValidWhenValidateChainFromOnePeer) {
  EXPECT_CALL(storage, getPeers())
      .WillOnce(Return(std::vector<model::Peer>(1)));

  Block block;
  block.sigs.emplace_back();
  auto block_observable = rxcpp::observable<>::just(block);

  EXPECT_CALL(storage, apply(block, _)).WillOnce(Return(true));

  ASSERT_TRUE(validator.validateChain(block_observable, storage));
}
