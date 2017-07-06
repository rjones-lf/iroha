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
#include <iostream>
#include <string>
#include <map_queue/map_queue.hpp>

const int N = 10;
structure::MapQueue<int, std::string> cmap(N);
int is[] = {100, 321, 2, 4, 31, 32, 55, 66, 44, 88};
std::string vs[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};

TEST(MapQueue, initialize) {
  ASSERT_TRUE(cmap.max_size() == N);
}

TEST(MapQueue, add_and_getMaxKey) {
  // add and getMaxKey Test
  int maxi = 0;
  for (int i = 0; i < N; i++) {
    maxi = std::max(maxi, is[i]);
    cmap.set(is[i], vs[i]);
    ASSERT_TRUE(cmap.getMaxKey() == maxi);
  }
}

TEST(MapQueue, check_key_eq_value) {
  // check key = value Test
  for (int i = 0; i < N; i++) {
    ASSERT_TRUE(cmap[is[i]] == vs[i]);
  }
}

TEST(MapQueue, change_size){
  // change size Test
  ASSERT_TRUE(cmap.size() == N);
  cmap.set_cache_size(5);
  ASSERT_TRUE(cmap.size() == 5);

  ASSERT_TRUE(cmap.getMaxKey() == 88);
  for (int i = 0; i < 5; i++) {
    bool except_flag = false;
    try {
      cmap[is[i]];
    } catch (std::out_of_range& e) {
      except_flag = true;
    }
    ASSERT_TRUE(except_flag);
  }
  for (int i = 5; i < N; i++) {
    ASSERT_TRUE(cmap[is[i]] == vs[i]);
  }
}