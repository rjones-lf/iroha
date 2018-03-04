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

#include "backend/protobuf/common_objects/amount.hpp"

#include <gtest/gtest.h>

using boost::multiprecision::uint256_t;

const uint256_t kOne = 1;
/// each 64 bit part starts with 1
const uint256_t kExpectedValue =
    (kOne << (64 * 3)) | (kOne << (64 * 2)) | (kOne << 64) | kOne;

/**
 * @given protobuf amount object
 * @when shared_model proto object is constructed
 * @then int value of shared model object equals to initial protobuf value
 */
TEST(AmountTest, AmountIntValueInitialization) {
  iroha::protocol::Amount amount;

  amount.mutable_value()->set_first(kOne.template convert_to<uint64_t>());
  amount.mutable_value()->set_second(kOne.template convert_to<uint64_t>());
  amount.mutable_value()->set_third(kOne.template convert_to<uint64_t>());
  amount.mutable_value()->set_fourth(kOne.template convert_to<uint64_t>());

  shared_model::proto::Amount proto_amount(amount);

  ASSERT_EQ(proto_amount.intValue(), kExpectedValue);
}

/**
 * @given uint256_t value
 * @when proto value of amount is filled
 * @then int value of proto object equals uint256_t value
 */
TEST(AmountTest, ProtoValueFromIntConversion) {
  iroha::protocol::Amount amount;

  shared_model::proto::convertToProtoAmount(*amount.mutable_value(),
                                            kExpectedValue);

  ASSERT_EQ(amount.value().first(), kOne);
  ASSERT_EQ(amount.value().second(), kOne);
  ASSERT_EQ(amount.value().third(), kOne);
  ASSERT_EQ(amount.value().fourth(), kOne);
}
