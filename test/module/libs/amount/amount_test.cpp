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

#include <gtest/gtest.h>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <amount/amount.hpp>

class AmountTest : public testing::Test {};

TEST_F(AmountTest, TestBasic) {
  iroha::Amount a(123, 2);

  // check taking percentage
  auto b = a.percentage(50);
  ASSERT_EQ(b.to_string(), "0.61");

  // check summation
  auto c = a + b;
  ASSERT_EQ(c.to_string(), "1.84");

  auto d = a.percentage(c); // taking 1.84% of 1.23
  ASSERT_EQ(d.to_string(), "0.02");

  auto e = a.percentage(iroha::Amount(256)); // taking 256% of the amount == 3.1488
  // assuming that we round down
  ASSERT_EQ(e.to_string(), "3.14");

  // check move constructor
  iroha::Amount f(std::move(a));
  ASSERT_EQ(f.to_string(), "1.23");

  // check equals when different precisions
  ASSERT_EQ(iroha::Amount(110, 2), iroha::Amount(11,1));

  // check comparisons
  ASSERT_LT(iroha::Amount(110, 2), iroha::Amount(111, 2));
  ASSERT_LE(iroha::Amount(110, 2), iroha::Amount(111, 2));
  ASSERT_LE(iroha::Amount(111, 2), iroha::Amount(111, 2));

  ASSERT_GT(iroha::Amount(111, 2), iroha::Amount(110, 2));
  ASSERT_GE(iroha::Amount(111, 2), iroha::Amount(110, 2));
  ASSERT_GE(iroha::Amount(111, 2), iroha::Amount(111, 2));

  std::cout << iroha::Amount(222,2).percentage(iroha::Amount(1111, 1)).to_string() << std::endl;
}
