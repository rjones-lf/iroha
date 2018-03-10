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

#ifndef IROHA_SHARED_MODEL_HASH_HPP
#define IROHA_SHARED_MODEL_HASH_HPP

#include <boost/functional/hash.hpp>
#include "cryptography/blob.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * A special class for storing hashes. Main reason to introduce it is to
     * make difference between Hash which should represent a hashing result and
     * a generic Blob which should represent any binary data.
     */
    class Hash : public Blob {
     public:
      /**
       * To calculate hash used by some standard containers
       */
      struct Hasher {
        std::size_t operator()(const Hash &h) const {
          using boost::hash_value;
          using boost::hash_combine;

          std::size_t seed = 0;
          hash_combine(seed, hash_value(h.blob()));

          return seed;
        }
      };

      Hash() : Blob() {}
      explicit Hash(const std::string &hash) : Blob(hash) {}

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Hash")
            .append(Blob::hex())
            .finalize();
      }
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_HASH_HPP
