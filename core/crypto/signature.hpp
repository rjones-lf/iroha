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

#ifndef CORE_CRYPTO_SIGNATURE_HPP_
#define CORE_CRYPTO_SIGNATURE_HPP_

#include <string>
#include <memory>
#include <vector>

namespace signature {

  constexpr size_t PRI_KEY_SIZE = 64;
  constexpr size_t PUB_KEY_SIZE = 32;
  constexpr size_t SIG_SIZE = 64;
  constexpr size_t SEED_SIZE = 32;

  using byte_t = unsigned char;
  using byte_array_t = std::vector<byte_t>;

  struct KeyPair {
    byte_array_t publicKey;
    byte_array_t privateKey;

    KeyPair(byte_array_t &&pub,
            byte_array_t &&pri) : publicKey(std::move(pub)),
                                  privateKey(std::move(pri)) {}
  };

  std::string sign(const std::string &message, const KeyPair &keyPair);

  std::string sign(const std::string &message,
                   const std::string &publicKey_b64,
                   const std::string &privateKey_b64);

  byte_array_t sign(const std::string &message,
                    const byte_array_t &publicKey,
                    const byte_array_t &privateKey);

  bool verify(const std::string &signature_b64,
              const std::string &message,
              const std::string &publicKey_b64);

  bool verify(const std::string &signature,
              const byte_array_t &message,
              const byte_array_t &publicKey);

  KeyPair generateKeyPair();

};  // namespace signature

#endif  // CORE_CRYPTO_SIGNATURE_HPP_
