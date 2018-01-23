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

#include <gtest/gtest.h>
#include "common/types.hpp"

using namespace iroha;
using namespace std::string_literals;

/**
 * @given byte string
 * @when byte string was converted to blob_t which has same size
 * @then converted blob_t match the result we are expected
 */
TEST(FromStringTest, ConvertTo_blob_t) {
  ASSERT_NO_THROW(hash256_t::from_string(std::string(hash256_t::size(), 1)));
}

/**
 * @given byte string
 * @when byte string was converted to blob_t which has different size
 * @then invalid argument exception throws
 */
TEST(FromStringTest, SizeMismatch) {
  try {
    hash256_t::from_string(std::string(31, 0));
  } catch (const std::invalid_argument &e) {
    ASSERT_STREQ(
        "blob_t: 'input string' has incorrect "
        "length\n"
        "Expected: 32 bytes\n"
        "Actual: 31 bytes\n",
        e.what());
  } catch (...) {
    FAIL() << "Exception type doesn't match";
  }
}

/**
 * @given byte string, messages for tracability
 * @when byte string was converted to blob_t which has different size
 * @then invalid argument exception throws
 */
TEST(FromStringTest, SizeMismatchWithMessage) {
  try {
    hash256_t::from_string(std::string(33, 1),
                           "FromStringTest::SizeMismatch",
                           "invalid_size_string");
  } catch (const std::invalid_argument &e) {
    ASSERT_STREQ(
        "FromStringTest::SizeMismatch: 'invalid_size_string' has incorrect "
        "length\n"
        "Expected: 32 bytes\n"
        "Actual: 33 bytes\n",
        e.what());
  } catch (...) {
    FAIL() << "Exception type doesn't match";
  }
}
