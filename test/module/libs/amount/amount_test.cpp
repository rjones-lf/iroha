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

using namespace boost::multiprecision;

TEST_F(AmountTest, TestBasic) {
  amount::Amount a(123, 2);
  auto b = a.percentage(50);
  ASSERT_EQ(b.to_string(), "0.61");

  auto c = a + b;
  ASSERT_EQ(c.to_string(), "1.84");

  auto d = a.percentage(c);
  ASSERT_EQ(d.to_string(), "0.02");

  auto e = a.percentage(amount::Amount(256));
  ASSERT_EQ(e.to_string(), "3.14");
}
