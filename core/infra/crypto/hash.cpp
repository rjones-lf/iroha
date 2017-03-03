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
extern "C" {
#include <SimpleFIPS202.h>
}
#include <crypto/hash.hpp>
#include <string>

namespace hash {

static inline std::string digest_to_hexdigest(const unsigned char *digest,
                                       size_t size) {
  char code[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  std::string res = "";
  unsigned char front, back;
  for (unsigned int i = 0; i < size; i++) {
    front = (digest[i] & 0xF0) >> 4;
    back = digest[i] & 0xF;
    res += code[front];
    res += code[back];
  }
  return res;
}

std::string sha3_256_hex(std::string message) {
  const int sha256_size = 32;  // bytes
  unsigned char digest[sha256_size];

  SHA3_256(digest, reinterpret_cast<const unsigned char *>(message.c_str()),
           message.size());

  return digest_to_hexdigest(digest, sha256_size);
}

std::string sha3_512_hex(std::string message) {
  const int sha512_size = 64;  // bytes
  unsigned char digest[sha512_size];

  SHA3_512(digest, reinterpret_cast<const unsigned char *>(message.c_str()),
           message.size());

  return digest_to_hexdigest(digest, sha512_size);
}

}  // namespace hash
