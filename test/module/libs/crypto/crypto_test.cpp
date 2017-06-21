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

#include <crypto/crypto.hpp>
#include <gtest/gtest.h>
#include <functional>
#include <string>

using namespace iroha::crypto;

TEST(Crypto, Digests) {
  std::string z = "0123456789abcdef";
  auto q = hexdigest_to_digest(z);
  auto w = digest_to_hexdigest(q->data(), q->size());
  ASSERT_EQ(z, w) << "valid case even length, leading zero";

  z = "1123456789abcdef";
  q = hexdigest_to_digest(z);
  w = digest_to_hexdigest(q->data(), q->size());
  ASSERT_EQ(z, w) << "valid case even length, leading non-zero";

  z = "1123456789abcdef0";
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "valid case odd length, leading non-zero";

  z = "0123456789abcdef0";
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "valid case odd length, leading zero";

  z = "123456789abcdef0q";  // invalid q char
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "invalid case odd length, leading zero";

  z = "123456789abcdef0q";  // invalid q char
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "invalid case odd length, leading non-zero";

  z = "23456789abcdef0q";  // invalid q char
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "invalid case even length, leading non-zero";

  z = "03456789abcdef0q";  // invalid q char
  q = hexdigest_to_digest(z);
  ASSERT_EQ(q, nonstd::nullopt) << "invalid case even length, leading zero";

  auto d = std::vector<uint8_t>{0, 1,  2,  3,  4,  5,  6,  7, 8,
                                9, 10, 11, 12, 13, 14, 15, 16};
  auto a = digest_to_hexdigest(d.data(), d.size());
  ASSERT_EQ(a, "000102030405060708090a0b0c0d0e0f10") << "digest to hexdigest";
}


TEST(Crypto, Base64) {
  auto a = "aGVsbG8gd29ybGQgbXkgbmFtZSBpcyBib2dkYW4K";
  auto b = base64_decode(a);
  std::string data = "hello world my name is bogdan\n";
  std::vector<uint8_t> expected{data.begin(), data.end()};
  ASSERT_EQ(expected, b) << "simple decode";

  std::string c = "hello world my name is bogdan\n";
  auto d = base64_encode((const unsigned char*)c.data(), c.size());
  ASSERT_EQ(d, a) << "simple encode case";

  std::string q = "qweqweqwe";
  std::vector<uint8_t> m{q.begin(), q.end()};
  auto w = base64_encode((const unsigned char*)q.data(), q.size());
  auto e = base64_decode(w);
  ASSERT_EQ(m, e) << "encode then decode";
}


TEST(Crypto, Hash) {
  std::string test = "hello world";
  std::vector<uint8_t> out(512 / 8);

  ASSERT_FALSE(sha3_512(out.data(), (uint8_t*)test.data(), test.size()));

  std::vector<uint8_t> expected512{
      0x84, 0x00, 0x06, 0x65, 0x3e, 0x9a, 0xc9, 0xe9, 0x51, 0x17, 0xa1,
      0x5c, 0x91, 0x5c, 0xaa, 0xb8, 0x16, 0x62, 0x91, 0x8e, 0x92, 0x5d,
      0xe9, 0xe0, 0x04, 0xf7, 0x74, 0xff, 0x82, 0xd7, 0x07, 0x9a, 0x40,
      0xd4, 0xd2, 0x7b, 0x1b, 0x37, 0x26, 0x57, 0xc6, 0x1d, 0x46, 0xd4,
      0x70, 0x30, 0x4c, 0x88, 0xc7, 0x88, 0xb3, 0xa4, 0x52, 0x7a, 0xd0,
      0x74, 0xd1, 0xdc, 0xcb, 0xee, 0x5d, 0xba, 0xa9, 0x9a};

  ASSERT_EQ(out, expected512);
}
