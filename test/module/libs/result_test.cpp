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
#include <common/result.hpp>

using namespace iroha::result;
using error = std::string;
using value = std::string;
using res = Result<value, error>;
using namespace std::literals::string_literals;

res dosomething(bool isValue) {
  if (isValue) {
    return Ok("value"s);
  } else {
    return Error("error"s);
  }
}

TEST(Result, value_instantiation) {
  // constructor, value
  res r = dosomething(true);
  ASSERT_TRUE(r) << "constructor: should be value";

  // operator =
  r = dosomething(true);
  ASSERT_TRUE(r) << "operator=: should be value";

  // static function
  ASSERT_NO_THROW(res a = Ok("ok!"s)) << "fails on Ok";

  r = Ok("hello"s);
  ASSERT_TRUE(r);
  ASSERT_EQ(r.ok(), "hello"s);
}

TEST(Result, error_instantiation) {
  // constructor, value
  res r = dosomething(false);
  ASSERT_FALSE(r) << "constructor: should be error";

  // operator =
  r = dosomething(false);
  ASSERT_FALSE(r) << "operator=: should be error";

  // static function
  ASSERT_NO_THROW(res a = Error("error"s)) << "fails on Error";

  r = Error("error"s);
  ASSERT_FALSE(r);
  ASSERT_EQ(r.error(), "error"s);
}

using R = Result<int, std::string>;
R increment_but_less_5(int a) {
  if (a < 5) {
    return Ok(a + 1);
  } else {
    return Error(":("s);
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

TEST(Result, pattern_matching) {
  R a = 1;
  ASSERT_TRUE(a);

  a.match([](const Ok_t<int> &v) { SUCCEED(); },
          [](const Error_t<std::string> &e) { FAIL(); });

  // create own visitor
  auto ok_handler = [](const R::OkType &v) { SUCCEED(); };
  auto error_handler = [](const R::ErrorType &v) { FAIL(); };
  auto visitor = make_visitor(ok_handler, error_handler);
  a.match(visitor);
}

TEST(Result, constant_result) {
  const R a = Error("test"s);
  ASSERT_EQ("test"s, a.error());
}
