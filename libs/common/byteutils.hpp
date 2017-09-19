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

#ifndef IROHA_BYTEUTILS_H
#define IROHA_BYTEUTILS_H

#include <string>

#include <nonstd/optional.hpp>

#include "crypto/base64.hpp"
extern "C" {
#include "crypto/lookup3.h"
}
#include "common/types.hpp"

namespace iroha {

  /**
   * Create blob_t from string of specified size
   * @tparam size - size of blob_t, expected size of string
   * @param s - string to convert
   * @return blob, if conversion was successful, otherwise nullopt
   */
  template<size_t size>
  nonstd::optional<blob_t<size>> stringToBlob(const std::string &string) {
    if (size != string.size()) {
      return nonstd::nullopt;
    }
    blob_t<size> array;
    std::copy(string.begin(), string.end(), array.begin());
    return array;
  }

  /**
   * Convert hexstring to array of given size
   * @tparam size - output array size
   * @param string - input string for transform
   * @return array of given size if size matches, nullopt otherwise
   */
  template <size_t size>
  nonstd::optional<blob_t<size>> hexstringToArray(const std::string &string) {
    return hexstringToBytestring(string) | stringToBlob<size>;
  }
}

// extend namespace std with custom hashing function for public key
namespace std {

  template <>
  struct hash<iroha::ed25519::pubkey_t> {
    size_t operator()(const iroha::ed25519::pubkey_t &pub) const {
      return hashlittle(pub.data(), pub.size(), 1337);
    }
  };
}

#endif  // IROHA_BYTEUTILS_H
