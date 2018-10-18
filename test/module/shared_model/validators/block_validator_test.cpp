/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/shared_model/validators/validators_fixture.hpp"

#include <gtest/gtest.h>

#include "validators/default_validator.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

class BlockValidatorTest : public ValidatorTest {
  auto getBaseBlockBuilder() const {
    auto tx = TestUnsignedTransactionBuilder()
        .creatorAccountId("account@domain")
        .setAccountQuorum("account@domain", 1)
        .createdTime(iroha::time::now())
        .quorum(1)
        .build()
        .signAndAddSignature(key)
        .finish();
    return shared_model::proto::TemplateBlockBuilder<
        (1 << shared_model::proto::TemplateBlockBuilder<>::total) - 1,
        shared_model::validation::AlwaysValidValidator,
        shared_model::proto::UnsignedWrapper<
            shared_model::proto::Block>>()
        .height(1)
        .prevHash(kPrevHash)
        .createdTime(iroha::time::now())
        .transactions(std::vector<decltype(tx)>{tx});

  shared_model::validation::DefaultUnsignedBlockValidator validator_;
};

/**
 * @given block validator @and valid non-empty block
 * @when block is validated
 * @then result is OK
 */
TEST_F(BlockValidatorTest, ValidBlock) {

}

/**
 * @given block validator @and empty block
 * @when block is validated
 * @then result is OK
 */
TEST_F(BlockValidatorTest, EmptyBlock) {

}

/**
 * @given block validator @and invalid block
 * @when block is validated
 * @then error appears after validation
 */
TEST_F(BlockValidatorTest, InvalidBlock) {

}
