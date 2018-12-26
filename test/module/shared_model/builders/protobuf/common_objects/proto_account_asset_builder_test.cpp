/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "module/shared_model/builders/protobuf/common_objects/proto_account_asset_builder.hpp"

/**
 * @given fields for AccountAsset object
 * @when AccountAssetBuilder is invoked
 * @then AccountAsset object is successfully constructed and has the same fields
 * as provided
 */
TEST(ProtoAccountAssetBuilder, AllFieldsBuild) {
  shared_model::proto::AccountAssetBuilder builder;

  auto expected_account_id = "account@name";
  auto expected_asset_id = "asset#coin";
  auto expected_balance = shared_model::interface::Amount("1.00");

  auto account_asset = builder.accountId(expected_account_id)
                           .assetId(expected_asset_id)
                           .balance(expected_balance)
                           .build();

  EXPECT_EQ(account_asset.accountId(), expected_account_id);
  EXPECT_EQ(account_asset.assetId(), expected_asset_id);
  EXPECT_EQ(account_asset.balance(), expected_balance);
}

/**
 * @given fields for AccountAsset object
 * @when AccountAssetBuilder is invoked twice with the same configuration
 * @then Two constructed AccountAsset objects are identical
 */
TEST(ProtoAccountAssetBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::proto::AccountAssetBuilder builder;

  auto expected_account_id = "account@name";
  auto expected_asset_id = "asset#coin";
  auto expected_balance = shared_model::interface::Amount("1.00");

  auto state = builder.accountId(expected_account_id)
                   .assetId(expected_asset_id)
                   .balance(expected_balance);

  auto account_asset = state.build();
  auto account_asset2 = state.build();

  EXPECT_EQ(account_asset.accountId(), account_asset2.accountId());
  EXPECT_EQ(account_asset.assetId(), account_asset2.assetId());
  EXPECT_EQ(account_asset.balance(), account_asset2.balance());
}
