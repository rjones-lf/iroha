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

#include "utils/amount_utils.hpp"

using namespace shared_model::detail;

class AmountTest : public testing::Test {};

/**
 * @given two amounts
 * @when tries to sum it up
 * @then correct amount is given
 */
TEST_F(AmountTest, PlusTest) {
  shared_model::interface::Amount a("1234.567"), b("100");

  auto c = a + b;
  c.match(
      [](const iroha::expected::Value<
          std::shared_ptr<shared_model::interface::Amount>> &c_value) {
        ASSERT_EQ(c_value.value->intValue(), 1334567);
        ASSERT_EQ(c_value.value->precision(), 3);
      },
      [](const iroha::expected::Error<std::shared_ptr<std::string>> &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given two amounts
 * @when tries to subtract it up
 * @then correct amount is given
 */
TEST_F(AmountTest, MinusTest) {
  shared_model::interface::Amount a("1234.567"), b("100");
  auto c = a - b;
  c.match(
      [](const iroha::expected::Value<
          std::shared_ptr<shared_model::interface::Amount>> &c_value) {
        ASSERT_EQ(c_value.value->intValue(), 1134567);
        ASSERT_EQ(c_value.value->precision(), 3);
      },
      [](const iroha::expected::Error<std::shared_ptr<std::string>> &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given an amount
 * @when tries change its precision
 * @then correct amount is given
 */
TEST_F(AmountTest, PrecisionTest) {
  shared_model::interface::Amount a("1234.567");
  auto c = makeAmountWithPrecision(a, 4);
  c.match(
      [](const iroha::expected::Value<
          std::shared_ptr<shared_model::interface::Amount>> &c_value) {
        ASSERT_EQ(c_value.value->intValue(), 12345670);
        ASSERT_EQ(c_value.value->precision(), 4);
      },
      [](const iroha::expected::Error<std::shared_ptr<std::string>> &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given two amounts which sum overflows amount type
 * @when tries to sum it up
 * @then an error occurs
 */
TEST_F(AmountTest, PlusOverflowsTest) {
  const std::string &uint256_halfmax =
      "723700557733226221397318656304299424082937404160253525246609900049457060"
      "2495.0";  // 2**252 - 1

  shared_model::interface::Amount a(uint256_halfmax), b("100.00");
  auto c = a + b;
  c.match(
      [](const iroha::expected::Value<
          std::shared_ptr<shared_model::interface::Amount>> &c_value) {
        FAIL() << "Operation successful but shouldn't";
      },
      [](const iroha::expected::Error<std::shared_ptr<std::string>> &e) {
        SUCCEED() << *e.error;
      });
}

/**
 * @given two amounts
 * @when tries to sum it up
 * @then correct amount is given
 */
TEST_F(AmountTest, LeadingZeroPlusTest) {
  shared_model::interface::Amount a("12.3"), b("03.21");

  auto c = a + b;
  c.match(
      [](const iroha::expected::Value<
          std::shared_ptr<shared_model::interface::Amount>> &c_value) {
        ASSERT_EQ(c_value.value->intValue(), 1551);
        ASSERT_EQ(c_value.value->precision(), 2);
      },
      [](const iroha::expected::Error<std::shared_ptr<std::string>> &e) {
        FAIL() << *e.error;
      });
}
