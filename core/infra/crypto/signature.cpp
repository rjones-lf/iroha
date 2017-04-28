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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include <ed25519.h>

#include <crypto/base64.hpp>
#include <crypto/signature.hpp>

namespace signature {

std::string sign(const std::string &message, const KeyPair &keyPair) {
  byte_array_t pub(keyPair.publicKey.begin(), keyPair.publicKey.end());
  byte_array_t pri(keyPair.privateKey.begin(), keyPair.privateKey.end());
  return base64::encode(sign(message, pub, pri));
}

std::string sign(const std::string &message, const std::string &publicKey_b64,
                 const std::string &privateKey_b64) {
  auto pub_decoded = base64::decode(publicKey_b64);
  auto pri_decoded = base64::decode(privateKey_b64);
  return base64::encode(sign(message, pub_decoded, pri_decoded));
  // byte_array_t pub(pub_decoded.begin(), pub_decoded.end());
  // byte_array_t pri(pri_decoded.begin(), pri_decoded.end());
  // return base64::encode(sign(message, pub, pri));
}


byte_array_t sign(const std::string &message, const byte_array_t &publicKey,
                  const byte_array_t &privateKey) {
  byte_array_t signature(SIG_SIZE);
  ed25519_sign(signature.data(),
               reinterpret_cast<const byte_t *>(message.c_str()),
               message.size(), publicKey.data(), privateKey.data());
  return signature;
}

bool verify(const std::string &signature_b64, const std::string &message,
            const std::string &publicKey_b64) {
  return ed25519_verify(base64::decode(signature_b64).data(),
                        reinterpret_cast<const byte_t *>(message.c_str()),
                        message.size(), base64::decode(publicKey_b64).data());
}

bool verify(const byte_array_t &signature, const std::string &message,
            const byte_array_t &publicKey) {
  return ed25519_verify(signature.data(),
                        reinterpret_cast<const byte_t *>(message.c_str()),
                        message.size(), publicKey.data());
}

KeyPair generateKeyPair() {
  byte_array_t pub(PUB_KEY_SIZE);
  byte_array_t pri(PRI_KEY_SIZE);
  byte_array_t seed(SEED_SIZE);

  // ed25519_create_seed may return 1 in case if it can not open /dev/urandom
  if (ed25519_create_seed(seed.data()) == 1) {
    throw "can not get seed";
  }

  ed25519_create_keypair(pub.data(), pri.data(), seed.data());

  return KeyPair(std::move(pub), std::move(pri));
}

};  // namespace signature
