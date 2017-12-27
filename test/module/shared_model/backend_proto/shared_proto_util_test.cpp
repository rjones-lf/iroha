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

#include "backend/protobuf/util.hpp"
#include "block.pb.h"

#include <gtest/gtest.h>

using namespace iroha;
using namespace shared_model::proto;
using shared_model::crypto::toBinaryString;

/**
 * @given some protobuf object
 * @when making a blob from it
 * @then make sure that the deserialized from string is the same
 */
TEST(UtilTest, StringFromMakeBlob) {
  protocol::Header base, deserialized;
  base.set_created_time(100);
  auto blob = makeBlob(base);

  ASSERT_TRUE(deserialized.ParseFromString(toBinaryString(blob)));
  ASSERT_EQ(deserialized.created_time(), base.created_time());
}
