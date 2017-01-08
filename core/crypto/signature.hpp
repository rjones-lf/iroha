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

  struct KeyPair {
    std::vector<unsigned char> publicKey;
    std::vector<unsigned char> privateKey;
	
    KeyPair(
      std::vector<unsigned char>&& pub,
      std::vector<unsigned char>&& pri
    ):
      publicKey(std::move(pub)),
      privateKey(std::move(pri))
    {}
  };

  std::string sign(
    std::string message,
    KeyPair keyPair
  );

  std::string sign(
    std::string message,
    std::string publicKey_b64,
    std::string privateKey_b64
  );

  bool verify(
    const std::string signature_b64,
    const std::string message,
    const std::string publicKey_b64);

  KeyPair generateKeyPair();

};  // namespace signature

#endif  // CORE_CRYPTO_SIGNATURE_HPP_
