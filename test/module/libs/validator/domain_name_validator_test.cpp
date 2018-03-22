/**
 * Copyright AltPlus Inc., Ltd. 2017 All Rights Reserved.
 * http://en.altplus.co.jp
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

#include "validator/domain_name_validator.hpp"
#include <regex>
#include <gtest/gtest.h>

using namespace validator;

TEST(DomainNameValidatorTest, HandleValidDomainName) {
  EXPECT_TRUE(isValidDomainName("a"));
  EXPECT_TRUE(isValidDomainName("ab"));
  EXPECT_TRUE(isValidDomainName("abc"));
  EXPECT_TRUE(isValidDomainName("abc.efg"));
  EXPECT_TRUE(isValidDomainName("abc.efg.hij"));
  EXPECT_TRUE(isValidDomainName("u9EEA432F"));
  EXPECT_TRUE(isValidDomainName("a-hyphen"));
  EXPECT_TRUE(isValidDomainName("altplus.com"));
  EXPECT_TRUE(isValidDomainName("altplus.com.jp"));
  EXPECT_TRUE(isValidDomainName(
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad"));
  EXPECT_TRUE(isValidDomainName("endWith0"));
}

TEST(DomainNameValidatorTest, HandleInvalidDomainName) {
  EXPECT_FALSE(isValidDomainName(" "));
  EXPECT_FALSE(isValidDomainName("9start.with.non.letter"));
  EXPECT_FALSE(isValidDomainName("-startWithDash"));
  EXPECT_FALSE(isValidDomainName("@.is.not.allowed"));
  EXPECT_FALSE(isValidDomainName("no space is allowed"));
  EXPECT_FALSE(isValidDomainName("endWith-"));
  EXPECT_FALSE(isValidDomainName("label.endedWith-.is.not.allowed"));
  EXPECT_FALSE(isValidDomainName(
      "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters"));
}
