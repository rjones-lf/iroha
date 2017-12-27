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

#include "utils/reference_holder.hpp"

#include <gtest/gtest.h>

/**
 * @given lvalue
 * AND corresponding reference holder
 * @when value is modified
 * @then reference holder dereference returns new value
 */
TEST(ReferenceHolder, lvalue) {
  int i{2};
  shared_model::detail::ReferenceHolder<int> rh(i);
  ASSERT_EQ(*rh, i);
  i = 3;
  ASSERT_EQ(*rh, i);
}

/**
 * @given rvalue
 * AND corresponding reference holder
 * @when value is modified
 * @then reference holder dereference returns new value
 */
TEST(ReferenceHolder, rvalue) {
  shared_model::detail::ReferenceHolder<int> rh(2);
  ASSERT_EQ(*rh, 2);
  *rh = 3;
  ASSERT_EQ(*rh, 3);
}

/**
 * @given rvalue
 * AND corresponding const reference holder
 * @when value is accessed
 * @then reference holder dereference returns stored value
 */
TEST(ReferenceHolder, ConstAccess) {
  const shared_model::detail::ReferenceHolder<int> rh(2);
  ASSERT_EQ(*rh, 2);
}
