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

#include "builders/protobuf/common_objects/proto_asset_builder.hpp"

TEST(ProtoAssetBuilderTest, AllFieldsBuild) {
  shared_model::proto::AssetBuilder builder;

  auto expected_asset_id = "asset@coin";
  auto expected_domain_id = "domain";
  auto expected_precision = 2;

  auto asset = builder.assetId(expected_asset_id).domainId(expected_domain_id).precision(expected_precision).build();

  EXPECT_EQ(asset.assetId(), expected_asset_id);
  EXPECT_EQ(asset.domainId(), expected_domain_id);
  EXPECT_EQ(asset.precision(), expected_precision);
}
