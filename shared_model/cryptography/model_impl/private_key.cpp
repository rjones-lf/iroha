/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "cryptography/private_key.hpp"

#include "utils/string_builder.hpp"

namespace shared_model {
  namespace crypto {

    PrivateKey::PrivateKey(const std::string &private_key)
        : Blob(private_key) {}

    PrivateKey::PrivateKey(const Blob &blob) : Blob(blob.blob()) {}

    std::string PrivateKey::toString() const {
      return detail::PrettyStringBuilder()
          .init("PrivateKey")
          .append("<Data is hidden>")
          .finalize();
    }
  }  // namespace crypto
}  // namespace shared_model
