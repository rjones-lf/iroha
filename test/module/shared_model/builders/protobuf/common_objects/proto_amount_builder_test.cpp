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

#include <boost/multiprecision/cpp_int.hpp>

#include "builders/protobuf/common_objects/proto_amount_builder.hpp"

/**
 * @given fields for Amount object
 * @when AmountBuilder is invoked
 * @then Amount object is successfully constructed and has the same fields as
 * provided
 */
TEST(ProtoAmountBuilderTest, AllFieldsBuild) {
  shared_model::proto::AmountBuilder builder;

  boost::multiprecision::uint256_t expected_int_value = 100;
  auto expected_precision = 2;

  auto amount = builder.intValue(expected_int_value)
                    .precision(expected_precision)
                    .build();

  EXPECT_EQ(amount.intValue(), expected_int_value);
  EXPECT_EQ(amount.precision(), expected_precision);
}

/**
 * @given fields for Amount object
 * @when AmountBuilder is invoked twice with the same configuration
 * @then Two constructed Amount objects are identical
 */
TEST(ProtoAmountBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::proto::AmountBuilder builder;

  boost::multiprecision::uint256_t expected_int_value{"123456789012345678901234567890"};
  auto expected_precision = 2;

  auto state =
      builder.intValue(expected_int_value).precision(expected_precision);
  auto amount = state.build();
  auto amount2 = state.build();

  EXPECT_EQ(amount.intValue(), amount2.intValue());
  EXPECT_EQ(amount.precision(), amount2.precision());
}
