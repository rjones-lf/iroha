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

#include "builders/common_objects/asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

TEST(AssetBuilderTest, StatelessValidAllFields) {

  shared_model::builder::AssetBuilder<shared_model::proto::AssetBuilder,
                                       shared_model::validation::FieldValidator>
      builder;

  auto valid_asset_id = "bit#connect";
  auto valid_domain_id = "domain";
  auto valid_precision = 2;

  auto asset = builder.assetId(valid_asset_id).domainId(valid_domain_id).precision(valid_precision).build();

  asset.match(
      [&](shared_model::builder::BuilderResult<shared_model::interface::Asset>::ValueType &v) {
        EXPECT_EQ(v.value->assetId(), valid_asset_id);
        EXPECT_EQ(v.value->domainId(), valid_domain_id);
        EXPECT_EQ(v.value->precision(), valid_precision);
      },
      [](shared_model::builder::BuilderResult<shared_model::interface::Asset>::ErrorType &e) { FAIL() << *e.error; });
}
