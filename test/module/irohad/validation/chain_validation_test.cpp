/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/specified_visitor.hpp"
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

    EXPECT_CALL(*block, height()).WillRepeatedly(Return(1));
    EXPECT_CALL(*block, prevHash()).WillRepeatedly(testing::ReturnRef(hash));
    EXPECT_CALL(*block, signatures())
        .WillRepeatedly(
            testing::Return(std::initializer_list<SignatureMock>{}));
    EXPECT_CALL(*block, payload()).WillRepeatedly(testing::ReturnRef(payload));
  }

  shared_model::crypto::Blob payload{"blob"};

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;

  std::shared_ptr<iroha::consensus::yac::MockSupermajorityChecker>
      supermajority_checker =
          std::make_shared<iroha::consensus::yac::MockSupermajorityChecker>();
  shared_model::crypto::Hash hash = shared_model::crypto::Hash("valid hash");
  std::shared_ptr<ChainValidatorImpl> validator;
  std::shared_ptr<MockMutableStorage> storage;
  std::shared_ptr<MockWsvQuery> query;
  std::shared_ptr<BlockMock> block = std::make_shared<BlockMock>();
  shared_model::interface::BlockVariant block_variant = block;
};

/**
 * @given valid block signed by peers
 * @when apply block
 * @then block is validated
 */
TEST_F(ChainValidationTest, ValidCase) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  EXPECT_CALL(*supermajority_checker,
              hasSupermajority(block_variant.signatures(), _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, check(testing::Ref(block_variant), _))
      .WillOnce(
          InvokeArgument<1>(ByRef(block_variant), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateBlock(block_variant, *storage));
}

/**
 * @given block with wrong hash
 * @when apply block
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  // Invalid previous hash, has supermajority, correct peers subset => invalid
  shared_model::crypto::Hash another_hash =
      shared_model::crypto::Hash(std::string(32, '1'));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, check(testing::Ref(block_variant), _))
      .WillOnce(InvokeArgument<1>(
          ByRef(block_variant), ByRef(*query), ByRef(another_hash)));

  ASSERT_FALSE(validator->validateBlock(block_variant, *storage));
}

/**
 * @given block with wrong peers
 * @when supermajority is not achieved
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenNoSupermajority) {
  // Valid previous hash, no supermajority, correct peers subset => invalid
  EXPECT_CALL(*supermajority_checker,
              hasSupermajority(block_variant.signatures(), _))
      .WillOnce(Return(false));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, check(testing::Ref(block_variant), _))
      .WillOnce(
          InvokeArgument<1>(ByRef(block_variant), ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block_variant, *storage));
}

/**
 * @given valid block signed by peer
 * @when apply block
 * @then block is validated via observer
 */
TEST_F(ChainValidationTest, ValidWhenValidateChainFromOnePeer) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  EXPECT_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillOnce(Return(true));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  // due to conversion to BlockVariant in validateChain() its impossible to pass
  // the block it will check() from here
  EXPECT_CALL(*storage, check(_, _))
      .WillOnce(
          InvokeArgument<1>(ByRef(block_variant), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateChain(
      rxcpp::observable<>::just(boost::apply_visitor(
          framework::SpecifiedVisitor<
              std::shared_ptr<shared_model::interface::Block>>(),
          block_variant)),
      *storage));
}
