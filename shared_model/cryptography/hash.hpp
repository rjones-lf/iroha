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

#include "cryptography/blob.hpp"

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
        std::size_t operator()(const Hash &h) const;
      };

      Hash();

      explicit Hash(const std::string &hash);

      explicit Hash(const Blob &blob);

      /**
       * Creates new Hash object from provided hex string
       * @param hex - string in hex format to create Hash from
       * @return Hash from provided hex string if it was correct or
       * Hash from empty string if provided string was not a correct hex string
       */
      static Hash fromHexString(const std::string &hex);

      std::string toString() const override;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_HASH_HPP
