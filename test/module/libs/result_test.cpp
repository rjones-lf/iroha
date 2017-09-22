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
#include <common/operators.hpp>
#include <common/result.hpp>

using namespace iroha;
using error = std::string;
using value = std::string;
using res = result<value, error>;
using namespace std::literals::string_literals;

res dosomething(bool isValue) {
  if (isValue) {
    return iroha::Ok("value"s);
  } else {
    return iroha::Error("error"s);
  }
}

TEST(Result, value_instantiation) {
  // costructor, value
  res r = dosomething(true);
  ASSERT_TRUE(r) << "constructor: should be value";

  // operator =
  r = dosomething(true);
  ASSERT_TRUE(r) << "operator=: should be value";

  // static function
  ASSERT_NO_THROW(res a = iroha::Ok("ok!"s)) << "fails on Ok";

  r = iroha::Ok("hello"s);
  ASSERT_TRUE(r);
  ASSERT_EQ(r.ok(), "hello"s);
}

TEST(Result, error_instantiation) {
  // costructor, value
  res r = dosomething(false);
  ASSERT_FALSE(r) << "constructor: should be error";

  // operator =
  r = dosomething(false);
  ASSERT_FALSE(r) << "operator=: should be error";

  // static function
  ASSERT_NO_THROW(res a = iroha::Error("error"s)) << "fails on Error";

  r = iroha::Error("error"s);
  ASSERT_FALSE(r);
  ASSERT_EQ(r.error(), "error"s);
}

using R = iroha::result<int, std::string>;
R increment_but_less_5(int a) {
  if (a < 5) {
    return Ok(a + 1);
  } else {
    return iroha::Error(":("s);
  }
}

TEST(Result, monadic) {
  R a = 1;
  ASSERT_TRUE(a);

  R b = a | increment_but_less_5 | increment_but_less_5;

  ASSERT_TRUE(a);
  ASSERT_TRUE(b);
  ASSERT_EQ(*b, *a + 2);

  R c = b | increment_but_less_5 | increment_but_less_5 | increment_but_less_5;
  ASSERT_FALSE(c);
  ASSERT_EQ(c.error(), ":("s);
}
