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
#include <primitive.pb.h>
#include <crypto/hash.hpp>

using namespace iroha;
namespace proto = iroha::protocol;

#define STR \
  "hello world this is my secret string which is not really secret ha ha"

TEST(PROTO, IsSignatureForDifferentProtoDifferent) {
  proto::Signature sig1, sig2, sig3, sig4, sig5, sig6, sig7, sig8;

  // first
  sig1.set_pubkey(STR);
  sig1.set_signature(STR);
  // second
  sig2.set_pubkey(STR);
  sig2.set_signature(STR);
  // copy from
  sig3.CopyFrom(sig1);
  // merge from
  sig4.MergeFrom(sig2);
  // initialization in reverse order.
  sig5.set_signature(STR);
  sig5.set_pubkey(STR);
  // extra text then set correct string
  sig6.set_pubkey(STR "and some extra space");
  sig6.set_signature(STR "and some extra space");
  sig6.set_pubkey(STR);
  sig6.set_signature(STR);
  // extra text then clean then correct string
  sig7.set_pubkey(STR "with extra space");
  sig7.set_pubkey(STR "with extra space");
  sig7.clear_pubkey();
  sig7.clear_signature();
  sig7.set_pubkey(STR);
  sig7.set_signature(STR);
  // parse from string
  auto str = sig7.SerializeAsString();
  sig8.ParseFromString(str);

  // calculate the hash of each string and compare them, they must be equal
  auto hash1 = sha3_256(sig1.SerializeAsString());
  auto hash2 = sha3_256(sig2.SerializeAsString());
  auto hash3 = sha3_256(sig3.SerializeAsString());
  auto hash4 = sha3_256(sig4.SerializeAsString());
  auto hash5 = sha3_256(sig5.SerializeAsString());
  auto hash6 = sha3_256(sig6.SerializeAsString());
  auto hash7 = sha3_256(sig7.SerializeAsString());
  auto hash8 = sha3_256(sig8.SerializeAsString());

  ASSERT_EQ(hash1, hash2) << "hash1 and hash2 are not equal";
  ASSERT_EQ(hash2, hash3) << "hash2 and hash3 are not equal";
  ASSERT_EQ(hash3, hash4) << "hash3 and hash4 are not equal";
  ASSERT_EQ(hash4, hash5) << "hash4 and hash5 are not equal";
  ASSERT_EQ(hash5, hash6) << "hash5 and hash6 are not equal";
  ASSERT_EQ(hash6, hash7) << "hash6 and hash7 are not equal";
  ASSERT_EQ(hash7, hash8) << "hash7 and hash8 are not equal";
}