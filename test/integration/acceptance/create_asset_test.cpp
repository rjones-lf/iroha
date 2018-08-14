/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits>

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;

class CreateAssetFixture : public AcceptanceFixture {
 public:
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kCreateAsset}) {
    return AcceptanceFixture::makeUserWithPerms(perms);
  }

  const interface::types::AssetNameType kAssetName = "newcoin";
  const interface::types::PrecisionType kPrecision = 1;
  const interface::types::PrecisionType kNonDefaultPrecision = kPrecision + 17;
  const interface::types::DomainIdType kNonExistingDomain = "nonexisting";
  const std::vector<interface::types::AssetNameType> kIllegalAssetNames = {
      "",
      " ",
      "   ",
      "A",
      "assetV",
      "asSet",
      "asset%",
      "^123",
      "verylongassetname_thenameislonger",
      "verylongassetname_thenameislongerthanitshouldbe",
      "assset-01"};

  const std::vector<interface::types::DomainIdType> kIllegalDomainNames = {
      "",
      " ",
      "   ",
      "9start.with.digit",
      "-startWithDash",
      "@.is.not.allowed",
      "no space is allowed",
      "endWith-",
      "label.endedWith-.is.not.allowed",
      "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters",
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP",
      "257.257.257.257",
      "domain#domain",
      "asd@asd",
      "ab..cd"};
};

/*
 * With the current implementation of CreateAsset method of TransactionBuilder
 * that is not possible to create tests for the following cases:
 *
 */

/**
 * @given some user with can_create_asset permission
 * @when the user tries to create an asset
 * @then asset is successfully created
 */
TEST_F(CreateAssetFixture, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(kAssetName, kDomain, kPrecision)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .done();
}

/**
 * C235 Create asset with an empty name
 * C236 Create asset with boundary values per name validation
 * @given some user with can_create_asset permission
 * @when the user tries to create an asset with invalid or empty name
 * @then no asset is created
 */
TEST_F(CreateAssetFixture, IllegalCharactersInName) {
  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  for (const auto &name : kIllegalAssetNames) {
    itf.sendTx(complete(baseTx().createAsset(name, kDomain, kPrecision)),
               checkStatelessInvalid);
  }
  itf.done();
}

/**
 * C234 Create asset with an existing id (name)
 * @given a user with can_create_asset permission
 * @when the user tries to create asset that already exists
 * @then stateful validation failed
 */
TEST_F(CreateAssetFixture, ExistingName) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(
          IntegrationTestFramework::kAssetName, kDomain, kPrecision)))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * C234a Create asset with an existing id (name) but different precision
 * @given a user with can_create_asset permission
 * @when the user tries to create asset that already exists but with different
 * precision
 * @then stateful validation failed
 */
TEST_F(CreateAssetFixture, ExistingNameDifferentPrecision) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(
          IntegrationTestFramework::kAssetName, kDomain, kNonDefaultPrecision)))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * C239 CreateAsset without such permissions
 * @given a user without can_create_asset permission
 * @when the user tries to create asset
 * @then stateful validation is failed
 */
TEST_F(CreateAssetFixture, WithoutPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({}))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(kAssetName, kDomain, kPrecision)))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given a user with can_create_asset permission
 * @when the user tries to create asset in valid but non existing domain
 * @then stateful validation will be failed
 */
TEST_F(CreateAssetFixture, ValidNonExistingDomain) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(
          complete(baseTx().createAsset(kAssetName, kNonExistingDomain, kPrecision)))
      .checkProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given a user with can_create_asset permission
 * @when the user tries to create an asset in a domain with illegal characters
 * @then stateless validation failed
 */
TEST_F(CreateAssetFixture, InvalidDomain) {
  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  for (const auto &domain : kIllegalDomainNames) {
    itf.sendTx(complete(baseTx().createAsset(kAssetName, domain, kPrecision)),
               checkStatelessInvalid);
  }
  itf.done();
}

/**
 * C237 Create asset with a negative precision
 * DISABLED because the current implementation of TransactionBuilder does not
 * allow to pass negative value on a type level
 * @given a user with can_create_asset permission
 * @when the user tries to create an asset with negative precision
 * @then stateless validation failed
 */
TEST_F(CreateAssetFixture, DISABLED_NegativePrecision) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(kAssetName, kDomain, -10)),
              checkStatelessInvalid)
      .done();
}

/**
 * C238 Create asset with overflow of precision data type
 * DISABLED because the current implementation of TransactionBuilder does not
 * allow to pass oversized value on a type level.
 * @given a user with can_create_asset permission
 * @when the user tries to create an asset with overflowed value of precision
 * @then stateless validation failed
 */
TEST_F(CreateAssetFixture, DISABLED_PrecisionOverflow) {
  uint64_t more_than_allowed = UCHAR_MAX + 1;
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTx(complete(baseTx().createAsset(
                  kAssetName,
                  kDomain,
                  (interface::types::PrecisionType)more_than_allowed)),
              checkStatelessInvalid)
      .done();
}
