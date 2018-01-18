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

#include "validator/address_validator.hpp"
#include <gtest/gtest.h>

/**
 * @given valid ip v4 address
 * @when is valid ipv4 is called
 * @then true is returned
 */
TEST(AddressValidatorTest, ValidIpV4AddressTest) {
  auto valid_addresses = {
      "0.0.0.0:0", "255.255.255.255:65535", "192.168.0.1:8080"};
  for (std::string valid_address : valid_addresses) {
    ASSERT_TRUE(iroha::validator::isValidIpV4(valid_address));
  }
}

/**
 * @given invalid ip v4 address
 * @when is valid ipv4 is called
 * @then false is returned
 */
TEST(AddressValidatorTest, InvalidIpV4AddressTest) {
  auto invalid_addresses = {
      "-0.0.0.0:0", "256.256.256.255:65535", "192.168.0.1:65536"};
  for (std::string invalid_address : invalid_addresses) {
    ASSERT_FALSE(iroha::validator::isValidIpV4(invalid_address));
  }
}

/**
 * @given valid hostname address
 * @when is valid hostname is called
 * @then true is returned
 */
TEST(AddressValidatorTest, ValidHostnameTest) {
  auto valid_addresses = {
      "abc.efg:0",
      "abc.efg.hij:65535",
      "a-hyphen.ru-i:8080",
      "altplus.com.jp:80",
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad:8080"};
  for (std::string valid_address : valid_addresses) {
    ASSERT_TRUE(iroha::validator::isValidHostname(valid_address));
  }
}

/**
 * @given invalid hostname address
 * @when is valid hostname is called
 * @then false is returned
 */
TEST(AddressValidatorTest, InvalidHostnameTest) {
  auto invalid_addresses = {
      "9.start.with.non.letter:0",
      "-startWithDash:65535",
      "@.is.not.allowed:8080",
      "no space is allowed:80",
      "too.big.port:65536",
      "some\u2063host:123",
      "endWith-:909",
      "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters:9090",
      // 256 character domain
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP:256",
      "",
      ":6565",
      "::6565:"
  };
  for (std::string invalid_address : invalid_addresses) {
    ASSERT_FALSE(iroha::validator::isValidHostname(invalid_address));
  }
}
