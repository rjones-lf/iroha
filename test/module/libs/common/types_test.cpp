/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <common/types.hpp>

using namespace iroha;

// len(PUB32) = 32 + \0, total = 33
#define PUB32 "11111111111111111111111111111111"

TEST(Blob, Construction) {
  using pub_t = blob_t<32>;

  ASSERT_NO_THROW(pub_t _p(PUB32)) << "constructor throws with correct length";

  ASSERT_THROW(pub_t _p(std::string("1")), std::invalid_argument)
      << "constructor does not throw with incorrect length";

  ASSERT_THROW(pub_t _p("1"), std::invalid_argument)
      << "constructor does not throw with incorrect length";

  ASSERT_NO_THROW(pub_t _p) << "default constructor throws";

  ASSERT_NO_THROW(auto p = pub_t::from_string(PUB32))
      << "from_string throws, but shouldn't";

  ASSERT_THROW(auto p = pub_t::from_string("1"), std::invalid_argument)
      << "from_string should throw, but does not";

  ASSERT_NO_THROW(auto p = pub_t::from_string_var("1"))
      << "from_string_var should not throw, but throws";

  {
    pub_t p = pub_t::from_string_var("2");
    std::string _s(pub_t::size(), '\0');
    _s[0] = '2';  // 2000000...00

    ASSERT_EQ(p, _s) << "from_string_var is wrong";
  }

  ASSERT_NO_THROW(pub_t p = std::string(PUB32))
      << "operator = throws, but should not";

  // with prefix 0x
  ASSERT_NO_THROW(
      pub_t p = pub_t::from_hexstring(
          "0x1212121212121212121212121212121212121212121212121212121212121212"))
      << "from_hexstring throws, but should not";

  // with prefix 0X
  ASSERT_NO_THROW(
      pub_t p = pub_t::from_hexstring(
          "0X1212121212121212121212121212121212121212121212121212121212121212"))
      << "from_hexstring throws, but should not";

  // without prefix 0x
  ASSERT_NO_THROW(
      pub_t p = pub_t::from_hexstring(
          "1212121212121212121212121212121212121212121212121212121212121212"))
      << "from_hexstring throws, but should not";

  // bad length (34)
  ASSERT_THROW(
      pub_t p = pub_t::from_hexstring(
          "121212121212121212121212121212121212121212121212121212121212121234"),
      std::invalid_argument)
      << "from_hexstring does not throw, but should.";

  // bad characters but good length
  ASSERT_THROW(
      pub_t p = pub_t::from_hexstring(
          "12121212121212121212121212121212121212121212121212121212121212zz"),
      std::invalid_argument)
      << "from_hexstring does not throw, but should.";

  // incorrect length
  ASSERT_THROW(pub_t p = pub_t::from_hexstring("123"), std::invalid_argument)
      << "from_hexstring does not throw, but should.";
}