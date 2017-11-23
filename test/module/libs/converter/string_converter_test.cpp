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
#include "common/types.hpp"

using namespace iroha;

TEST(StringConverterTest, ConvertHexToBinary) {
  std::string hex = "ff000233551117daa110050399";
  std::string bin = "\xFF\x02\x33\x55\x11\x17\xDA\xA1\x10\x05\x03\x99";
  bin.insert(bin.begin() + 1, '\0');
  ASSERT_EQ(hexstringToBytestring(hex).value(), bin);
}

TEST(StringConverterTest, ConvertBinaryToHex) {
  std::string bin = "\xFF\x02\x33\x55\x11\x17\xDA\xA1\x10\x05\x03\x99";
  bin.insert(bin.begin() + 1, '\0');
  ASSERT_EQ(bytestringToHexstring(bin), "ff000233551117daa110050399");
}

TEST(StringConverterTest, ConvertHexToBinaryAndBack) {
  std::string hexString = "0123456789abcdef";
  ASSERT_EQ(hexString,
            bytestringToHexstring(hexstringToBytestring(hexString).value()));
}
