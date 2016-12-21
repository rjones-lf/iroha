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

#include "../../crypto/hash.hpp"


#include <SimpleFIPS202.h>
#include <string>

namespace hash {

  std::string sha3_256_hex(std::string message) {
    char code[] =
      {'0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    unsigned char output[64];

    SHA3_256(
      reinterpret_cast<unsigned char *>(output),
      reinterpret_cast<const unsigned char *>(message.c_str()),
      reinterpret_cast<size_t>(message.size())
    );

    std::string res = "";
    unsigned char front, back;
    for (int i = 0; i < 32; i++) {
      front = (output[i] & 240) >> 4;
      back =  output[i] &  15;
      res += code[front];
      res += code[back];
    }
    return res;
  }

  std::string sha3_512_hex(std::string message) {
    char code[] =
      {'0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    unsigned char output[128];
    SHA3_512(
      output,
      (const unsigned char *)message.c_str(),
      message.size()
    );

    std::string res = "";
    unsigned char front, back;
    for (int i = 0; i < 64; i++) {
      front = (output[i] & 240) >> 4;
      back =  output[i] &  15;
      res += code[front];
      res += code[back];
    }
    return res;
  }

}  // namespace hash
