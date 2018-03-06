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

#include "backend/protobuf/from_old_model.hpp"
#include "builders/common_objects/peer_builder.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/irohad/model/model_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "validation/impl/chain_validator_impl.hpp"
#include "validators/field_validator.hpp"

using namespace iroha;
using namespace iroha::model;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

using ::testing::A;
using ::testing::ByRef;
using ::testing::InvokeArgument;
using ::testing::Return;
using ::testing::_;

auto zero_string = std::string(32, '\0');
auto fake_hash = shared_model::crypto::Hash(zero_string);

class ChainValidationTest : public ::testing::Test {
 public:
  void SetUp() override {
    validator = std::make_shared<ChainValidatorImpl>(supermajority_checker_);
    storage = std::make_shared<MockMutableStorage>();
    query = std::make_shared<MockWsvQuery>();
  }

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;
  Block block;
  shared_model::interface::types::HashType hash{fake_hash};

  std::shared_ptr<iroha::consensus::yac::MockSupermajorityChecker>
      supermajority_checker_ =
          std::make_shared<iroha::consensus::yac::MockSupermajorityChecker>();

  std::shared_ptr<ChainValidatorImpl> validator;
  std::shared_ptr<MockMutableStorage> storage;
  std::shared_ptr<MockWsvQuery> query;
};

/**
 * @given valid block signed by peers
 * @when apply block
 * @then block is validated
 */
TEST_F(ChainValidationTest, ValidCase) {
  EXPECT_CALL(*supermajority_checker_, hasSupermajority(_, _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  auto old_block = shared_model::proto::from_old(block);
  EXPECT_CALL(*storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(old_block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateBlock(block, *storage));
}

/**
 * @given block with wrong hash
 * @when apply block
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  hash = shared_model::interface::types::HashType(
      std::string(1, block.prev_hash.size()));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  auto old_block = shared_model::proto::from_old(block);
  EXPECT_CALL(*storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(old_block), ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

/**
 * @given block with wrong peers
 * @when supermajority is not achieved
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenNoSupermajority) {
  EXPECT_CALL(*supermajority_checker_, hasSupermajority(_, _))
      .WillOnce(Return(false));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  auto old_block = shared_model::proto::from_old(block);
  EXPECT_CALL(*storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(old_block), ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

/**
 * @given valid block chain signed by peers
 * @when apply block chain
 * @then block chain is validated
 */
TEST_F(ChainValidationTest, ValidWhenValidateChainFromOnePeer) {
  EXPECT_CALL(*supermajority_checker_, hasSupermajority(_, _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  auto block_observable = rxcpp::observable<>::just(block);

  auto old_block = shared_model::proto::from_old(block);
  EXPECT_CALL(*storage, apply(_, _))
      .WillOnce(InvokeArgument<1>(ByRef(old_block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateChain(block_observable, *storage));
}
