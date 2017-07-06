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


#include <datetime/time.hpp>
#include <gtest/gtest.h>

TEST(Time, Now32) {
  auto time = iroha::time::now32();
  std::cout << "Unix timestamp32: " << time << std::endl;

  ASSERT_GT(time, 1497447645);  // 06/14/2017 @ 1:40pm (UTC)
  ASSERT_EQ(sizeof(time), 4);
}

TEST(Time, Now64) {
  auto time = iroha::time::now64();
  std::cout << "Unix timestamp64: " << time << std::endl;

  ASSERT_GT(time, 1497447645);  // 06/14/2017 @ 1:40pm (UTC)
  ASSERT_EQ(sizeof(time), 8);
}
