/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <ametsuchi/currency.h>
#include <gtest/gtest.h>

using ametsuchi::Currency;

TEST(Currency_Test, ComparisonOperatorTest) {
  uint64_t amount1 = 1234;
  uint8_t precision1 = 2;

  uint64_t amount2 = 1231;
  uint8_t precision2 = 2;

  Currency c1(amount1, precision1);
  Currency c2(amount2, precision2);

  ASSERT_TRUE(c2 < c1);
  ASSERT_FALSE(c1 < c2);

  ASSERT_TRUE(c1 > c2);
  ASSERT_FALSE(c2 > c1);
}

TEST(Currency_Test, AddTest_Test) {
  uint64_t amount1 = 1234;
  uint8_t precision1 = 2;

  uint64_t amount2 = 1231;
  uint8_t precision2 = 2;

  Currency c1(amount1, precision1);
  Currency c2(amount2, precision2);

  Currency res = c1 + c2;

  ASSERT_EQ(res.get_amount(), 2465);
  ASSERT_EQ(res.get_precision(), 2);
  ASSERT_EQ(res.integer(), 24);
  ASSERT_EQ(res.fractional(), 65);

  Currency pi(314, 2);
  Currency e(273, 2);

  res = pi + e;
  ASSERT_EQ(res.get_amount(), 587);
  ASSERT_EQ(res.get_precision(), 2);
  ASSERT_EQ(res.integer(), 5);
  ASSERT_EQ(res.fractional(), 87);
}

TEST(Currency_Test, SubtractTest) {
  uint64_t amount1 = 1234;
  uint8_t precision1 = 2;

  uint64_t amount2 = 1231;
  uint8_t precision2 = 2;

  Currency c1(amount1, precision1);
  Currency c2(amount2, precision2);

  Currency res = c1 - c2;

  ASSERT_EQ(res.get_amount(), 3);
  ASSERT_EQ(res.get_precision(), 2);
  ASSERT_EQ(res.integer(), 0);
  ASSERT_EQ(res.fractional(), 3);

  Currency pi(314, 2);
  Currency e(273, 2);

  res = pi - e;
  ASSERT_EQ(res.get_amount(), 41);
  ASSERT_EQ(res.get_precision(), 2);
  ASSERT_EQ(res.integer(), 0);
  ASSERT_EQ(res.fractional(), 41);
}