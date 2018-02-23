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

#include "builders/common_objects/amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders_test_fixture.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

/**
 * @given field values which pass stateless validation
 * @when  AmountBuilder is invoked
 * @then Amount object is successfully constructed and has valid fields
 */
TEST(AmountBuilderTest, StatelessValidAllFields) {
  shared_model::builder::AmountBuilder<shared_model::proto::AmountBuilder,
                                       shared_model::validation::FieldValidator>
      builder;

  boost::multiprecision::uint256_t valid_value = 100;
  auto valid_precision = 2;

  auto amount =
      builder.intValue(valid_value).precision(valid_precision).build();

  amount.match(
      [&](shared_model::builder::BuilderResult<
          shared_model::interface::Amount>::ValueType &v) {
        EXPECT_EQ(v.value->intValue(), valid_value);
        EXPECT_EQ(v.value->precision(), valid_precision);
      },
      [](shared_model::builder::BuilderResult<
          shared_model::interface::Amount>::ErrorType &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given field values which pass stateless validation
 * @when AmountBuilder is invoked twice
 * @then Two identical (==) Amount objects are constructed
 */
TEST(AmountBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::builder::AmountBuilder<shared_model::proto::AmountBuilder,
                                       shared_model::validation::FieldValidator>
      builder;

  boost::multiprecision::uint256_t valid_value = 100;
  auto valid_precision = 2;

  auto state = builder.intValue(valid_value).precision(valid_precision);

  auto amount = state.build();
  auto amount2 = state.build();

  testResultObjects(amount, amount2, [](auto &a, auto &b) {
    // pointer points to different objects
    ASSERT_TRUE(a != b);

    EXPECT_EQ(a->intValue(), b->intValue());
    EXPECT_EQ(a->precision(), b->precision());
  });
}
