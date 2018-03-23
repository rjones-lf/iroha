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

#include "cryptography/blob.hpp"

#include "common/byteutils.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace crypto {

    std::string toBinaryString(const Blob &b) {
      return std::string(b.blob().begin(), b.blob().end());
    }

    Blob::Blob(const std::string &blob)
        : Blob(Bytes(blob.begin(), blob.end())) {}

    Blob::Blob(const Bytes &blob) : Blob(Bytes(blob)) {}

    Blob::Blob(Bytes &&blob) noexcept : blob_(std::move(blob)) {
      hex_ = iroha::bytestringToHexstring(toBinaryString(*this));
    }

    Blob *Blob::clone() const {
      return new Blob(blob());
    }

    bool Blob::operator==(const Blob &rhs) const {
      return blob() == rhs.blob();
    }

    Blob Blob::fromHexString(const std::string &hex) {
      using iroha::operator|;
      Blob b("");
      iroha::hexstringToBytestring(hex) | [&](auto &&s) { b = Blob(s); };
      return b;
    }

    const Blob::Bytes &Blob::blob() const {
      return blob_;
    }

    const std::string &Blob::hex() const {
      return hex_;
    }

    size_t Blob::size() const {
      return blob_.size();
    }

    std::string Blob::toString() const {
      return detail::PrettyStringBuilder()
          .init("Blob")
          .append(hex())
          .finalize();
    }

  }  // namespace crypto
}  // namespace shared_model
