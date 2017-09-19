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

#include "common/byteutils.hpp"
#include "common/types.hpp"
#include "crypto/base64.hpp"
#include "crypto/crypto.hpp"

#include <gtest/gtest.h>

using iroha::create_seed;
using iroha::create_keypair;
using iroha::sign;
using iroha::verify;
namespace ed25519 = iroha::ed25519;
using iroha::stringToBlob;

TEST(Signature, sign_data_size) {
  auto seed = create_seed();
  auto keypair = create_keypair(seed);

  std::string nonce =
      "c0a5cca43b8aa79eb50e3464bc839dd6fd414fae0ddf928ca23dcebf8a8b8dd0";
  auto signature = sign((const unsigned char*)nonce.c_str(), nonce.size(),
                        keypair.pubkey, keypair.privkey);

  ASSERT_TRUE(verify((const unsigned char*)nonce.c_str(), nonce.size(),
                     keypair.pubkey, signature));
}

TEST(Signature, PrintkeyPair) {
  auto seed = create_seed();
  auto keypair = create_keypair(seed);
  ASSERT_NO_THROW({ std::cout << keypair.pubkey.to_base64() << std::endl; });
  ASSERT_NO_THROW({ std::cout << keypair.privkey.to_base64() << std::endl; });
}

TEST(Signature, generatedByAndroid) {
  std::string private_key_b64 =
      "+BTZfRSPRgDdxmjZlK+QhJ3RQryMH23LIPqg5C/"
      "Eu2QcBoj3QM6ovTcmPok0iFYI1y9M683ZS4Ifp10jr9dQrQ==";
  std::string public_key_b64 = "b+etgin9x1S16omALSjr4HTVzv9IEXQzlvSTp7el0Js=";
  std::string signature_b64 =
      "HlJIjuds2OaSeyOjWjpnpXis55NvH3TD1SNVEwedu7sAY+Ypkksg3ovHUGfBhwd8uVmIX+"
      "JgnjrhKgPdyeO7DA==";
  std::string message_ =
      "0f1a39c82593e8b48e69f000c765c8e8072269d3bd4010634fa51d4e685076e30db22a9f"
      "b75def7379be0e808392922cb8c43d5dd5d5039828ed7ade7e1c6c81";
  std::vector<uint8_t> message(message_.begin(), message_.end());

  auto pubkey_ = base64_decode(public_key_b64);
  auto pubkey = stringToBlob<ed25519::pubkey_t::size()>(
      std::string{pubkey_.begin(), pubkey_.end()});

  ed25519::sig_t signature;
  std::vector<uint8_t> signature_v = base64_decode(signature_b64);
  ASSERT_EQ(signature.size(), signature_v.size());
  std::copy(signature_v.begin(), signature_v.end(), signature.begin());

  ASSERT_TRUE(pubkey.has_value());
  ASSERT_TRUE(
      verify(message.data(), message.size(), pubkey.value(), signature));
}

TEST(Signature, generatedByiOS) {
  std::string public_key_b64 = "slyr7oz2+EU6dh2dY9+jNeO/hVrXCkT3rGhcNZo5rrE=";
  std::string signature_b64 =
      "gdMUgjyo++4QpF1xDJNdk1a5zmDAEPM67WD4cn6CVZqDxC8nShb/"
      "L1Tokgo53HSOPDB0qXAVzcBvfcJ1WLjrAQ==";
  std::string message_ =
      "46ed8c250356759f68930a94996faaa8f8c98ecbe0dcc58c479c8fad71e30096";
  std::vector<uint8_t> message(message_.begin(), message_.end());

  auto pubkey_ = base64_decode(public_key_b64);
  auto pubkey = stringToBlob<ed25519::pubkey_t::size()>(
      std::string{pubkey_.begin(), pubkey_.end()});

  ed25519::sig_t signature;
  std::vector<uint8_t> signature_v = base64_decode(signature_b64);
  ASSERT_EQ(signature.size(), signature_v.size());
  std::copy(signature_v.begin(), signature_v.end(), signature.begin());

  ASSERT_TRUE(pubkey.has_value());
  ASSERT_TRUE(
      verify(message.data(), message.size(), pubkey.value(), signature));
}

TEST(Signature, generatedByGO) {
  std::string public_key_b64 = "Oe8Xd5iDvVrVNc1apzLcdjRRq3ZZsTjkjebpDYdRcRw";
  std::string signature_b64 =
      "FtLGaJLDK4g/"
      "tRzufBexe6fTAhjENtl6MWAynRpR9c1CZdEWJbbDS9svpU96hXiyGy3BQcwRxUz6eovBJdf6"
      "DQ==";
  std::string message_ =
      "0f1a39c82593e8b48e69f000c765c8e8072269d3bd4010634fa51d4e685076e30db22a9f"
      "b75def7379be0e808392922cb8c43d5dd5d5039828ed7ade7e1c6c81";
  std::vector<uint8_t> message(message_.begin(), message_.end());

  auto pubkey_ = base64_decode(public_key_b64);
  auto pubkey = stringToBlob<ed25519::pubkey_t::size()>(
      std::string{pubkey_.begin(), pubkey_.end()});

  ed25519::sig_t signature;
  std::vector<uint8_t> signature_v = base64_decode(signature_b64);
  ASSERT_EQ(signature.size(), signature_v.size());
  std::copy(signature_v.begin(), signature_v.end(), signature.begin());

  ASSERT_TRUE(pubkey.has_value());
  ASSERT_TRUE(
      verify(message.data(), message.size(), pubkey.value(), signature));
}
