/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"
#include "validation/impl/chain_validator_impl.hpp"

using namespace iroha;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

using ::testing::_;
using ::testing::A;
using ::testing::ByRef;
using ::testing::InvokeArgument;
using ::testing::Return;

class ChainValidationTest : public ::testing::Test {
 public:
  void SetUp() override {
    validator = std::make_shared<ChainValidatorImpl>(supermajority_checker);
    storage = std::make_shared<MockMutableStorage>();
    query = std::make_shared<MockWsvQuery>();
    peers = std::vector<std::shared_ptr<shared_model::interface::Peer>>();
  }

  auto setupBlock(BlockMock &block) const {
    EXPECT_CALL(block, height()).WillRepeatedly(Return(1));
    EXPECT_CALL(block, prevHash()).WillRepeatedly(testing::ReturnRef(hash));
    EXPECT_CALL(block, signatures()).WillRepeatedly(testing::Return(sigs));
    EXPECT_CALL(block, payload()).WillRepeatedly(testing::ReturnRef(payload));
  }

  std::array<SignatureMock, 0> sigs;
  shared_model::crypto::Blob payload = shared_model::crypto::Blob("blob");

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;

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
  BlockMock block;
  setupBlock(block);

  EXPECT_CALL(*supermajority_checker, hasSupermajority(block.signatures(), _))
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
  BlockMock block;
  setupBlock(block);

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
  BlockMock block;
  setupBlock(block);

  EXPECT_CALL(*supermajority_checker, hasSupermajority(block.signatures(), _))
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
  auto block = std::make_shared<BlockMock>();
  setupBlock(*block);

  EXPECT_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(testing::Ref(*block), _))
      .WillOnce(InvokeArgument<1>(ByRef(*block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateChain(
      rxcpp::observable<>::just(
          std::static_pointer_cast<shared_model::interface::Block>(block)),
      *storage));
}
