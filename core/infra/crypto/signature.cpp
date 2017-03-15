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

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cstring>
#include <algorithm>

#include <ed25519.h>

#include <crypto/signature.hpp>
#include <crypto/base64.hpp>
#include <crypto/hash.hpp>

#include <cassert>

namespace signature {

template<typename T>
std::vector<T> vector2UnsignedCharPointer(const std::vector<T> &vec) {
  std::vector<T> res(vec.size() + 1);
  std::copy(vec.begin(), vec.end(), res.begin());
  res[res.size() - 1] = '\0';
  return res;
}

template<typename T>
std::vector<T> pointer2Vector(std::unique_ptr<T[]>&& array, size_t length) {
    std::vector<T> res(length);
    res.assign(array.get(),array.get()+length);
    return res;
}

std::string sign(const std::string &message,
                 const KeyPair &keyPair) {
  std::vector<unsigned char> pub(keyPair.publicKey.begin(), keyPair.publicKey.end());
  std::vector<unsigned char> pri(keyPair.privateKey.begin(), keyPair.privateKey.end());
  return base64::encode(sign(message, pub, pri));
}

std::string sign(const std::string &message,
                 const std::string &publicKey_b64,
                 const std::string &privateKey_b64) {
  auto pub_decoded = base64::decode(publicKey_b64);
  auto pri_decoded = base64::decode(privateKey_b64);
  std::vector<unsigned char> pub(pub_decoded.begin(), pub_decoded.end());
  std::vector<unsigned char> pri(pri_decoded.begin(), pri_decoded.end());
  return base64::encode(sign(message, pub, pri));
}


std::vector<unsigned char> sign(const std::string &message,
                                const std::vector<unsigned char> &publicKey,
                                const std::vector<unsigned char> &privateKey) {
  std::vector<unsigned char> signature(SIG_SIZE);
  ed25519_sign(signature.data(),
             reinterpret_cast<const unsigned char*>(message.c_str()),
             message.size(),
             publicKey.data(),
             privateKey.data());
  return signature;
}

bool verify(const std::string &signature_b64,
            const std::string &message,
            const std::string &publicKey_b64) {
  return ed25519_verify(base64::decode(signature_b64).data(),
                        reinterpret_cast<const unsigned char*>(message.c_str()),
                        message.size(),
                        base64::decode(publicKey_b64).data());
}

bool verify(const std::vector<unsigned char> &signature,
            const std::string &message,
            const std::vector<unsigned char> &publicKey) {
  return ed25519_verify(signature.data(),
                        reinterpret_cast<const unsigned char*>(message.c_str()),
                        message.size(),
                        publicKey.data());
}

KeyPair generateKeyPair() {
  std::unique_ptr<unsigned char[]> publicKeyRaw(new unsigned char[sizeof(unsigned char)*32]);
  std::unique_ptr<unsigned char[]> privateKeyRaw(new unsigned char[sizeof(unsigned char)*64]);
  std::unique_ptr<unsigned char[]> seed(new unsigned char[sizeof(unsigned char)*32]);

  ed25519_create_seed(seed.get());
  ed25519_create_keypair(
    publicKeyRaw.get(),
    privateKeyRaw.get(),
    seed.get()
  );

  return KeyPair(
     pointer2Vector(std::move(publicKeyRaw), 32),
     pointer2Vector(std::move(privateKeyRaw), 64)
   );
}

};  // namespace signature
