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

#include "builders/common_objects/account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: Add mocks for template parameters
// TODO: Add more test cases

TEST(AccountAsset, StatelessValidAllFields) {
  shared_model::builder::AccountAssetBuilder<
      shared_model::proto::AccountAssetBuilder,
      shared_model::validation::FieldValidator>
      builder;

  auto valid_account_id = "account@name";
  auto valid_asset_id = "asset#coin";
  auto valid_balance =
      shared_model::proto::AmountBuilder().intValue(100).precision(2).build();

  auto account_asset = builder.accountId(valid_account_id)
                           .assetId(valid_asset_id)
                           .balance(valid_balance)
                           .build();

  account_asset.match(
      [&](shared_model::builder::BuilderResult<
          shared_model::interface::AccountAsset>::ValueType &v) {
        EXPECT_EQ(v.value->accountId(), valid_account_id);
        EXPECT_EQ(v.value->assetId(), valid_asset_id);
        EXPECT_EQ(v.value->balance(), valid_balance);
      },
      [](shared_model::builder::BuilderResult<
          shared_model::interface::AccountAsset>::ErrorType &e) {
        FAIL() << *e.error;
      });
}
