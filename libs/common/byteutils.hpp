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

#include <crypto/base64.hpp>
#include <string>
#include "types.hpp"

namespace iroha {

  /**
   * Converts given string to the blob of given size.
   * @tparam size
   * @param s
   * @return
   */
  template <size_t size>
  blob_t<size> to_blob(std::string s) {
    if (s.size() != size) throw std::runtime_error("to_blob size mismatch");

    blob_t<size> b;
    std::copy(s.begin(), s.end(), b.begin());

    return b;
  }
}

#endif  // IROHA_BYTEUTILS_H
