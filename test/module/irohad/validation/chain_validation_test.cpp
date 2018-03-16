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
    validator = std::make_shared<ChainValidatorImpl>(supermajority_checker);
    storage = std::make_shared<MockMutableStorage>();
    query = std::make_shared<MockWsvQuery>();
    peers = std::vector<std::shared_ptr<shared_model::interface::Peer>>();
  }

  /**
   * Get block builder to build blocks for tests
   * @return block builder
   */
  auto getBlockBuilder() const {
    return TestBlockBuilder()
        .transactions(std::vector<shared_model::proto::Transaction>{})
        .height(1)
        .prevHash(hash);
  }

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;
  Block block;

  std::shared_ptr<iroha::consensus::yac::MockSupermajorityChecker>
      supermajority_checker =
          std::make_shared<iroha::consensus::yac::MockSupermajorityChecker>();
  shared_model::crypto::Hash hash = shared_model::crypto::Hash("valid hash");
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
  // Valid previous hash, has supermajority, correct peers subset => valid
  auto block = getBlockBuilder().build();

  EXPECT_CALL(*supermajority_checker,
              hasSupermajority(testing::Ref(block.signatures()), _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(testing::Ref(block), _))
      .WillOnce(InvokeArgument<1>(ByRef(block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateBlock(block, *storage));
}

/**
 * @given block with wrong hash
 * @when apply block
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  // Invalid previous hash, has supermajority, correct peers subset => invalid
  auto block = getBlockBuilder().build();

  shared_model::crypto::Hash another_hash =
      shared_model::crypto::Hash(std::string(32, '1'));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(testing::Ref(block), _))
      .WillOnce(
          InvokeArgument<1>(ByRef(block), ByRef(*query), ByRef(another_hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

/**
 * @given block with wrong peers
 * @when supermajority is not achieved
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenNoSupermajority) {
  // Valid previous hash, no supermajority, correct peers subset => invalid
  auto block = getBlockBuilder().build();

  EXPECT_CALL(*supermajority_checker,
              hasSupermajority(testing::Ref(block.signatures()), _))
      .WillOnce(Return(false));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(testing::Ref(block), _))
      .WillOnce(InvokeArgument<1>(ByRef(block), ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

/**
 * @given valid block signed by peer
 * @when apply block
 * @then block is validated via observer
 */
TEST_F(ChainValidationTest, ValidWhenValidateChainFromOnePeer) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  auto block = getBlockBuilder().build();
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      block_observable = rxcpp::observable<>::just(block).map([](auto &&x) {
        return std::shared_ptr<shared_model::interface::Block>(x.copy());
      });

  EXPECT_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(
      *storage,
      apply(testing::Truly([&](const shared_model::interface::Block &rhs) {
              return rhs == block;
            }),
            _))
      .WillOnce(InvokeArgument<1>(ByRef(block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateChain(block_observable, *storage));
}
