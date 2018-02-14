/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include <gtest/gtest.h>

#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"

TEST(ProtoAccountAssetBuilder, AllFieldsBuild) {
  shared_model::proto::AccountAssetBuilder builder;

  auto expected_account_id = "account@name";
  auto expected_asset_id = "asset#coin";
  auto expected_balance = shared_model::proto::AmountBuilder().intValue(100).precision(2).build();

  auto account_asset = builder.accountId(expected_account_id).assetId(expected_asset_id).balance(expected_balance).build();

  EXPECT_EQ(account_asset.accountId(), expected_account_id);
  EXPECT_EQ(account_asset.assetId(), expected_asset_id);
  EXPECT_EQ(account_asset.balance(), expected_balance);
}
