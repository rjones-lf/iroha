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
using namespace std::literals::string_literals;

TEST(Result, value_instantiation) {
  using R = Result<bool, bool>;
  // constructor, value
  R r = true;
  ASSERT_TRUE(r) << "constructor: should be value";

  // operator =
  r = false;  // any value of type bool is Ok_t unless Error(bool) is specified
  ASSERT_TRUE(r) << "operator=: should be value";

  // static function
  ASSERT_NO_THROW(R a = Ok(false)) << "fails on Ok";

  r = Ok(true);
  ASSERT_TRUE(r);
  ASSERT_EQ(r.ok(), true);

  // const
  const R z = true;
  R x = z;
  const R c(std::move(x));
  const R v{c};
  ASSERT_TRUE(z);
  ASSERT_TRUE(c);
  ASSERT_TRUE(v);

  // get value
  ASSERT_NO_THROW(r.ok());
  ASSERT_TRUE(r.ok());  // r contains true in value
  // on const object
  ASSERT_NO_THROW(z.ok());
  ASSERT_TRUE(z.ok());  // z contains true in value
}

TEST(Result, error_instantiation) {
  using R = Result<bool, bool>;

  // constructor, value
  R r = Error(false);
  ASSERT_FALSE(r) << "constructor: should be error";

  // operator =
  r = Error(false);
  ASSERT_FALSE(r) << "operator=: should be error";

  const R z = Error(false);
  R x = z;
  const R c(std::move(x));
  const R v{c};
  ASSERT_FALSE(z);
  ASSERT_FALSE(c);
  ASSERT_FALSE(v);

  // get error value
  ASSERT_NO_THROW(r.error());
  ASSERT_FALSE(r.error());  // r contains false in error
  // on const object
  ASSERT_NO_THROW(z.error());
  ASSERT_FALSE(z.error());  // z contains false in error
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

  // monadic sequence test
  using Rm = Result<bool, bool>;
  auto succeed = [](auto &&) -> Rm {
    SUCCEED();
    return Ok(true);
  };

  auto make_error = [](auto &&) -> Rm { return Error(false); };

  auto fail = [](auto &&) -> Rm { ADD_FAILURE(); };

  // 3 consecutive success operations which return value, then 4th returns
  // error. if 5th is executed, than test should fail.
  Rm test_ = Ok(true);
  Rm r_ = test_ | succeed | succeed | succeed | make_error | fail;
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

TEST(Result, accessors_constructors) {
  using R = Result<std::string, std::string>;
  R nonc = "ok"s;
  ASSERT_TRUE(nonc) << "contains value, but false";
  ASSERT_EQ(*nonc, "ok"s);
  ASSERT_EQ(nonc->size(), (*nonc).size());

  const R c = "ok"s;
  ASSERT_TRUE(c) << "contains value, but false";

  auto ok = Ok("ok"s);
  R a1 = ok;
  R a2{ok};
  R a3(ok);
  R a4(std::move(ok));
  R a5(R(Ok("ok"s)));
  R a6 = Error("err"s);
  a6 = std::move(Ok("ok"s));

  const auto V = Ok("ok"s);
  R a7 = Error("err"s);
  a7 = V;
  ASSERT_TRUE(a1);
  ASSERT_TRUE(a2);
  ASSERT_TRUE(a3);
  ASSERT_TRUE(a4);
  ASSERT_TRUE(a5);
  ASSERT_TRUE(a6);
  ASSERT_EQ(*a1, "ok"s);

  auto err = Error("err"s);
  R b1 = err;
  R b2{err};
  R b3(err);
  R b4(std::move(err));
  R b5(R(Error("err"s)));
  R b6 = Ok("ok"s);
  b6 = std::move(Error("err"s));

  const auto E = Error("err"s);
  R b7 = Ok("ok"s);
  b7 = E;
  ASSERT_FALSE(b1);
  ASSERT_FALSE(b2);
  ASSERT_FALSE(b3);
  ASSERT_FALSE(b4);
  ASSERT_FALSE(b5);
  ASSERT_FALSE(b6);
  ASSERT_FALSE(b7);

  // rvalue operator * and ->
  ASSERT_TRUE(R("ok"s));
  ASSERT_EQ(*R("ok"s), "ok"s);
  ASSERT_EQ(R("ok"s)->size(), "ok"s.size());

  const R c1 = Ok("ok"s);
  ASSERT_FALSE(!c1);
  ASSERT_EQ(c1->size(), (*c1).size());

  // copy constructors
  R v1 = Ok("okay"s);
  R v2 = v1;
  R v3 = R(Ok("okay"s));
  R v4((R(Error("s"s))));
  ASSERT_TRUE(v1);
  ASSERT_TRUE(v2);
  ASSERT_TRUE(v3);
  ASSERT_FALSE(v4);
}
