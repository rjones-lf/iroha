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

#include <ametsuchi/merkle_tree/circular_stack.h>
#include <gtest/gtest.h>

namespace ametsuchi {
namespace buffer {

constexpr size_t size = 10;

TEST(CircularStack, IterForEach) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size; ++i) cs.push(i);
  ASSERT_TRUE(cs.begin() != cs.end());
  size_t i = 0;
  for (auto c = cs.begin(); c != cs.end(); ++c) {
    ASSERT_EQ(*c, i++);
  }
  ASSERT_EQ(i, size);
  i = 0;
  for (auto c : cs) {
    ASSERT_EQ(c, i++);
  }
  ASSERT_EQ(i, size);
  auto it = cs.begin();
  for (i = 0; i < size; ++i) {
    ASSERT_EQ(it[i], i);
  }
  ASSERT_EQ(i, size);
}

TEST(CircularStack, IterSumOverflow) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size + 1; ++i) cs.push(i);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(cs.begin() + (size + i), cs.end());
    if (i != size - 1) ASSERT_NE(cs.begin(), cs.end());
  }
}

TEST(CircularStack, IterThroughOneOverflow) {
  CircularStack<int> cs(size);
  for (size_t i = 0; i < size; ++i) cs.push(i);
  auto i = 0;
  for (auto c = cs.begin(); c != cs.end(); c += 3) {
    ASSERT_EQ(*c, i);
    i += 3;
  }
  ASSERT_EQ(i, size + 3 - (size % 3));
}

TEST(CircularStack, IterAccess) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size; ++i) cs.push(i);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(cs[i], i);
  }
}

TEST(CircularStack, IterLast) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(cs.last(), cs.begin().to_last());
    cs.push(i);
  }
  ASSERT_EQ(cs.last(), cs.begin().to_last());
}

}  // namespace buffer
}  // namespace ametsuchi
