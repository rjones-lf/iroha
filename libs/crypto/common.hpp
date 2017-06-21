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

#ifndef IROHA_COMMON_H
#define IROHA_COMMON_H

#include <nonstd/optional.hpp>
#include <string>
#include <vector>

namespace iroha {
  namespace crypto {
    static const std::string code = {'0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    /**
     * Standard lengths for ed25519. Keypair strongly depends on these values.
     */
    namespace ed25519 {
      const int8_t SIGNATURELEN = 64, PUBLEN = 32, PRIVLEN = 64;
    }

    /**
     * Convert hex string into bytes.
     * Returns nonstd::nullopt if input string has odd length!
     * @param str
     * @return
     */
    inline nonstd::optional<std::vector<uint8_t>> hexdigest_to_digest(
        std::string str) {
      // input data should be in hex
      size_t len = str.size();
      if (len % 2 != 0 || len == 0) return nonstd::nullopt;

      std::vector<uint8_t> res(str.size() / 2);

      size_t k = 0;
      auto it = str.begin();
      auto end = str.end();
      while (it != end) {
        for (char i = 0; i < 2; i++) {
          char a = *it;
          ++it;

          size_t pos = code.find(a);
          if (pos != std::string::npos) {
            // found
            res[k] |= (pos << (4 * (1 - i)));
          } else {
            // not found
            return nonstd::nullopt;
          }
        }

        k++;  // move to the next pair <0xXX>
      }

      return nonstd::optional<std::vector<uint8_t>>(res);
    }

    /**
     * Converts bytes to hex string.
     * @param digest
     * @param size
     * @return
     */
    inline std::string digest_to_hexdigest(const uint8_t *digest, size_t size) {
      std::string res = "";
      uint8_t front, back;
      for (uint32_t i = 0; i < size; i++) {
        front = (uint8_t)(digest[i] & 0xF0) >> 4;
        back = (uint8_t)(digest[i] & 0xF);
        res += code[front];
        res += code[back];
      }
      return res;
    }
  }
}
#endif  // IROHA_COMMON_H
