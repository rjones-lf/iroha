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
#include <string>
#include "utils/lazy_initializer.hpp"

struct SourceValue {
  int val;
};

struct TargetValue {
  std::string target;
};

/**
 * @given Initialized value
 * @when Call get once
 * @then Assert that transform invoked
 */
TEST(LazyTest, GetterTest) {
  SourceValue v{100500};
  auto lazy = shared_model::detail::makeLazyInitializer(
      [&v] { return TargetValue{std::to_string(v.val)}; });
  ASSERT_EQ("100500", lazy->target);
}

/**
 * @given Initialized value
 * @when Call get twice
 * @then Assert that transform invoked once
 */
TEST(LazyTest, CheckLaziness) {
  SourceValue v{100500};
  auto call_counter = 0;
  auto lazy = shared_model::detail::makeLazyInitializer([&call_counter, &v] {
    call_counter++;
    return TargetValue{std::to_string(v.val)};
  });
  ASSERT_EQ(0, call_counter);
  ASSERT_EQ("100500", lazy->target);
  ASSERT_EQ(1, call_counter);
  ASSERT_EQ("100500", lazy->target);
  ASSERT_EQ(1, call_counter);
}
