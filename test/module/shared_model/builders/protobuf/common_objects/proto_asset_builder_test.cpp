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

/**
 * @given fields for Asset object
 * @when AssetBuilder is invoked
 * @then Asset object is successfully constructed and has the same fields as
 * provided
 */
TEST(ProtoAssetBuilderTest, AllFieldsBuild) {
  shared_model::proto::AssetBuilder builder;

  auto expected_asset_id = "asset@coin";
  auto expected_domain_id = "domain";
  auto expected_precision = 2;

  auto asset = builder.assetId(expected_asset_id)
                   .domainId(expected_domain_id)
                   .precision(expected_precision)
                   .build();

  EXPECT_EQ(asset.assetId(), expected_asset_id);
  EXPECT_EQ(asset.domainId(), expected_domain_id);
  EXPECT_EQ(asset.precision(), expected_precision);
}

/**
 * @given fields for Asset object
 * @when AssetBuilder is invoked twice with the same configuration
 * @then Two constructed Asset objects are identical
 */
TEST(ProtoAssetBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::proto::AssetBuilder builder;

  auto expected_asset_id = "asset@coin";
  auto expected_domain_id = "domain";
  auto expected_precision = 2;

  auto state = builder.assetId(expected_asset_id)
                   .domainId(expected_domain_id)
                   .precision(expected_precision);
  auto asset = state.build();
  auto asset2 = state.build();

  EXPECT_EQ(asset.assetId(), asset2.assetId());
  EXPECT_EQ(asset.domainId(), asset2.domainId());
  EXPECT_EQ(asset.precision(), asset2.precision());
}
