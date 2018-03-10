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

#include "common/result.hpp"
#include <gtest/gtest.h>

using namespace iroha::expected;

constexpr auto kErrorCaseMessage = "Unexpected error case";
constexpr auto kValueCaseMessage = "Unexpected value case";
constexpr auto kFirstFunctionCallMessage = "first function";
constexpr auto kErrorMessage = "error message";

// Function which returns function which when invoked, fails the test
template <typename T>
auto makeFailCase(const std::string &error_message) {
  return [&error_message](T t) { FAIL() << error_message; };
}

// Function which, when invoked successfully passes test
auto correct = [](auto t) { SUCCEED(); };

/**
 * @given Result constructed with valid value
 * @when match function is invoked
 * @then match executes value case
 */
TEST(ResultTest, ResultValueConstruction) {
  Result<int, std::string> result = makeValue(5);
  result.match(correct, makeFailCase<Error<std::string>>(kErrorCaseMessage));
}

/**
 * @given Result constructed with error value
 * @when match function is invoked
 * @then match executes error case
 */
TEST(ResultTest, ResultErrorConstruction) {
  Result<int, std::string> result = makeError("error message");
  result.match(makeFailCase<Value<int>>(kValueCaseMessage), correct);
}

/**
 * @given Result constructed with the same type of value and error
 * @when match function is invoked
 * @then match executes successful case
 */
TEST(ResultTest, ResultSameTypeValueConstruction) {
  Result<int, int> result = makeValue(5);
  result.match(correct, makeFailCase<Error<int>>(kErrorCaseMessage));
}

/**
 * @given Result constructed with the same type of value and error
 * @when match function is invoked
 * @then match executes error case
 */
TEST(ResultTest, ResultSameTypeErrorConstruction) {
  Result<int, int> result = makeError(10);
  result.match(makeFailCase<Value<int>>(kValueCaseMessage), correct);
}

/**
 * @given Two functions which return result
 * @when bind operator is used to chain these 2 functions and they both return
 *       valid values
 * @then result of a bind contains valid value.
 */
TEST(ResultTest, ResultBindOperatorSuccesfulCase) {
  auto get_int = []() -> Result<int, std::string> { return makeValue(10); };
  auto negate_int = [](int a) -> Result<int, std::string> {
    return makeValue(-1 * a);
  };

  auto result = get_int() | negate_int;

  result.match([](Value<int> v) { ASSERT_EQ(-10, v.value); },
               makeFailCase<Error<std::string>>(kErrorCaseMessage));
}

// function which must not be called for test to pass
auto never_used_negate_int = [](int a) -> Result<int, std::string> {
  EXPECT_TRUE(false);
  return makeValue(-1 * a);
};

auto error_get_int = []() -> Result<int, std::string> {
  return makeError(kFirstFunctionCallMessage);
};

/**
 * @given two functions which return result
 * @when bind operator is used to chain these 2 functions and first one return
 *       error
 * @then result of bind contains error and second function is not invoked
 */
TEST(ResultTest, ResultBindOperatorErrorFirstFunction) {
  auto result = error_get_int() | never_used_negate_int;

  result.match(makeFailCase<Value<int>>(kValueCaseMessage),
               [](Error<std::string> e) {
                 ASSERT_EQ(kFirstFunctionCallMessage, e.error);
               });
}

/**
 * @given two functions which return different results, but value and error
 *        types can be cast to each other
 * @when bind operator is used to chain these 2 functions
 * @then result type is properly deduced
 */
TEST(ResultTest, ResultBindOperatorCompatibleTypes) {
  auto result = error_get_int() | never_used_negate_int;

  static_assert(
      std::is_same<decltype(result), decltype(never_used_negate_int(1))>::value,
      "Result type does not match function return type");

  result.match(makeFailCase<Value<int>>(kValueCaseMessage),
               [](Error<std::string> e) {
                 ASSERT_EQ(kFirstFunctionCallMessage, e.error);
               });
}

/**
 * @given Result with void value
 * @when match function is invoked
 * @then void type is correctly handled by compiler
 */
TEST(ResultTest, ResultVoidValue) {
  Result<void, std::string> result = makeError(kErrorMessage);
  result.match(makeFailCase<Value<void>>(kValueCaseMessage),
               [](Error<std::string> e) { ASSERT_EQ(kErrorMessage, e.error); });
}

/**
 * @given Result with void error
 * @when match function is invoked
 * @then void type is correctly handled by compiler
 */
TEST(ResultTest, ResultVoidError) {
  Result<int, void> result = makeValue(5);
  result.match([](Value<int> v) { ASSERT_EQ(5, v.value); },
               makeFailCase<Error<void>>(kErrorCaseMessage));
}

/// Polymorphic result tests

/// Base and Derived are classes, which can be used to test polymorphic behavior
class Base {
 public:
  virtual int getNumber() {
    return 0;
  }
};

class Derived : public Base {
 public:
  virtual int getNumber() override {
    return 1;
  }
};

/**
 * @given Polymorphic Result of Base class type with value of derived class
 * @when match function is invoked
 * @then Value case is invoked, and polymorphic behavior persists
 */
TEST(PolyMorphicResultTest, PolymorphicValueConstruction) {
  PolymorphicResult<Base, std::string> result =
      makeValue(std::make_shared<Derived>());
  result.match(
      [](Value<std::shared_ptr<Base>> &v) {
        ASSERT_EQ(1, v.value->getNumber());
      },
        makeFailCase<Error<std::shared_ptr<std::string>>>(kErrorCaseMessage));
}

/**
 * @given Polymorphic Result of Base class type with error
 * @when match function is invoked
 * @then Error case is invoked
 */
TEST(PolyMorphicResultTest, PolymorphicErrorConstruction) {
  PolymorphicResult<Base, std::string> result =
      makeError(std::make_shared<std::string>(kErrorMessage));
  result.match(
      makeFailCase<Value<std::shared_ptr<Base>>>(kValueCaseMessage),
      [](Error<std::shared_ptr<std::string>> &e) {
        ASSERT_EQ(kErrorMessage, *e.error);
      });
}
