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

#include "cryptography/hash.hpp"

#include <boost/functional/hash.hpp>

#include "utils/string_builder.hpp"

namespace shared_model {
  namespace crypto {

    Hash::Hash() : Blob() {}

    Hash::Hash(const std::string &hash) : Blob(hash) {}

    std::string Hash::toString() const {
      return detail::PrettyStringBuilder()
          .init("Hash")
          .append(Blob::hex())
          .finalize();
    }

    std::size_t Hash::Hasher::operator()(const Hash &h) const {
      using boost::hash_combine;
      using boost::hash_value;

      std::size_t seed = 0;
      hash_combine(seed, hash_value(h.blob()));

      return seed;
    }
  }  // namespace crypto
}  // namespace shared_model
