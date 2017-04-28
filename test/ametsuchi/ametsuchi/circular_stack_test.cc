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
#include <stack>

namespace ametsuchi {
namespace buffer {

constexpr size_t size = 10;

TEST(CircularStack, Addition) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size * size; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);
}

TEST(CircularStack, AdditionMovingStruct) {
  struct T {
    int i;
  };
  T t[] = {{1}, {2}, {3},  {4},  {5},  {5},  {6},  {7},
           {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15}};
  CircularStack<T> cs(size);
  for (size_t i = 0; i < sizeof(t) / sizeof(T); ++i) {
    cs.push(std::move(t[i]));
  }
}

TEST(CircularStack, Poping) {
  CircularStack<size_t> cs(size);

  for (size_t i = 0; i < size; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);

  cs.pop();
  ASSERT_EQ(cs.size(), size - 1);

  while (cs.size() > 0) cs.pop();
  ASSERT_EQ(cs.size(), 0);

  cs.pop();
  ASSERT_EQ(cs.size(), 0);


  for (size_t i = 0; i < size; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);

  cs.pop(size);
  ASSERT_EQ(cs.size(), 0);
}

TEST(CircularStack, Resizing) {
  size_t size = 10;
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size * size; ++i) cs.push(i);
  size *= 2;
  cs.resize(size);
  for (size_t i = 0; i < size * size; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);
}

TEST(CircularStack, Accessing) {
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(cs[i], i);
  }
}

TEST(CircularStack, AccessingOverflow) {
  CircularStack<uint64_t> cs(size);
  // 5..14
  for (size_t i = 0; i < size * 1.5; ++i) cs.push(i);
  ASSERT_EQ(cs.size(), size);
  ASSERT_EQ(cs.capacity(), size);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(cs[i], i + 5);
  }
}

TEST(CircularStack, EmptyDestruction) {
  CircularStack<uint64_t> cs(1);
  cs.~CircularStack();
}

TEST(CircularStack, ExceptionCatches) {
  try {
    CircularStack<uint64_t> cs(0);
  } catch (exception::Exception &e) {
    ASSERT_STREQ(e.what(), "Buffer size cannot be zero");
  }
  try {
    CircularStack<uint64_t> cs(1);
    cs[1];
  } catch (exception::Exception &e) {
    ASSERT_STREQ(e.what(), "Buffer accessing out of size");
  }
}

TEST(CircularStack, IterForEach) {
  try {
    CircularStack<uint64_t> cs(0);
  } catch (exception::Exception &e) {
    ASSERT_STREQ(e.what(), "Buffer size cannot be zero");
  }
  try {
    CircularStack<uint64_t> cs(1);
    cs[1];
  } catch (exception::Exception &e) {
    ASSERT_STREQ(e.what(), "Buffer accessing out of size");
  }
}

TEST(CirularStack, PushAndPopAndFront) {
  size_t size = 10;
  std::stack<size_t> st;
  CircularStack<size_t> cs(size);
  for (size_t i = 0; i < size - 1; i++) {
    st.push(i);
    cs.push(i);
  }

  for (size_t i = 0; i < 5; i++) {
    ASSERT_EQ(st.top(), cs.back());
    st.pop();
    cs.pop();
  }
  for (size_t i = 0; i < 5; i++) {
    st.push(i + size);
    cs.push(i + size);
  }
  while (!st.empty()) {
    ASSERT_EQ(st.top(), cs.back());
    st.pop();
    cs.pop();
  }
}

}  // namespace buffer
}  // namespace ametsuchi
